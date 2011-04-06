//
//  iosegmenter.c
//  iosegmenter
//
//  Created by Emil Palm on 4/5/11.
//  Copyright 2011 none. All rights reserved.
//

#include "ruby.h"
#include "libavformat/avformat.h"
#include "libavutil/log.h"

static AVStream *add_output_stream(AVFormatContext *output_format_context, AVStream *input_stream) {
    AVCodecContext *input_codec_context;
    AVCodecContext *output_codec_context;
    AVStream *output_stream;
    
    output_stream = av_new_stream(output_format_context, 0);
    if (!output_stream) {
        fprintf(stderr, "Could not allocate stream\n");
        exit(1);
    }
    
    input_codec_context = input_stream->codec;
    output_codec_context = output_stream->codec;
    
    output_codec_context->codec_id = input_codec_context->codec_id;
    output_codec_context->codec_type = input_codec_context->codec_type;
    output_codec_context->codec_tag = input_codec_context->codec_tag;
    output_codec_context->bit_rate = input_codec_context->bit_rate;
    output_codec_context->extradata = input_codec_context->extradata;
    output_codec_context->extradata_size = input_codec_context->extradata_size;
    
    if(av_q2d(input_codec_context->time_base) * input_codec_context->ticks_per_frame > av_q2d(input_stream->time_base) && av_q2d(input_stream->time_base) < 1.0/1000) {
        output_codec_context->time_base = input_codec_context->time_base;
        output_codec_context->time_base.num *= input_codec_context->ticks_per_frame;
    }
    else {
        output_codec_context->time_base = input_stream->time_base;
    }
    
    switch (input_codec_context->codec_type) {
        case CODEC_TYPE_AUDIO:
            output_codec_context->channel_layout = input_codec_context->channel_layout;
            output_codec_context->sample_rate = input_codec_context->sample_rate;
            output_codec_context->channels = input_codec_context->channels;
            output_codec_context->frame_size = input_codec_context->frame_size;
            if ((input_codec_context->block_align == 1 && input_codec_context->codec_id == CODEC_ID_MP3) || input_codec_context->codec_id == CODEC_ID_AC3) {
                output_codec_context->block_align = 0;
            }
            else {
                output_codec_context->block_align = input_codec_context->block_align;
            }
            break;
        case CODEC_TYPE_VIDEO:
            output_codec_context->pix_fmt = input_codec_context->pix_fmt;
            output_codec_context->width = input_codec_context->width;
            output_codec_context->height = input_codec_context->height;
            output_codec_context->has_b_frames = input_codec_context->has_b_frames;
            
            if (output_format_context->oformat->flags & AVFMT_GLOBALHEADER) {
                output_codec_context->flags |= CODEC_FLAG_GLOBAL_HEADER;
            }
            break;
        default:
            break;
    }
    
    return output_stream;
}


typedef struct _segment {
    int index;
    long duration;
    char *filename;
} Segment;

static VALUE rb_mAvSegmenter;
static VALUE rb_cAvSegmenter;
static VALUE rb_cAvSegment;

/* Segment */

static void segment_free(Segment *segment)
{

    if (NULL == segment)
        return;
    
    segment->index = 0;
    segment->duration = 0;
    segment->filename = NULL;
    ruby_xfree(segment);
}

static VALUE segment_allocate (VALUE klass)
{
    
    Segment *s = ALLOC(Segment);
    s->duration = 0;
    s->index = 0;
    s->filename = NULL;    
	return Data_Wrap_Struct (klass, NULL, segment_free, s);
}

static VALUE segment_init(VALUE self, VALUE index, VALUE duration, VALUE filename)
{
    rb_iv_set(self, "@index", index);
    rb_iv_set(self, "@duration", duration);
    rb_iv_set(self, "@filename", filename);
    return self;
}

/* Segmenter */

static VALUE segmenter_add_segment(VALUE self,VALUE segment)
{
    
    
    if ( NIL_P(rb_iv_get(self,"@segments")) )
    {
        rb_iv_set(self,"@segments",rb_ary_new());
    }
    
    VALUE sArray = rb_iv_get(self,"@segments");
    return rb_ary_push(sArray, segment);
}

static VALUE segmenter_segment(VALUE klass, VALUE input_, VALUE output_prefix_, VALUE duration_ ) {

    const char *input;
    const char *output_prefix;
    double segment_duration;
    char *segment_duration_check;
    const char *index;
    char *tmp_index;
    const char *http_prefix;
    long max_tsfiles = 0;
    char *max_tsfiles_check;
    double prev_segment_time = 0;
    unsigned int output_index = 1;
    AVInputFormat *ifmt;
    AVOutputFormat *ofmt;
    AVFormatContext *ic = NULL;
    AVFormatContext *oc;
    AVStream *video_st;
    AVStream *audio_st;
    AVCodec *codec;
    char *output_filename;
    char *remove_filename;
    int video_index;
    int audio_index;
    unsigned int first_segment = 1;
    unsigned int last_segment = 0;
    int write_index = 1;
    int decode_done;
    char *dot;
    int ret;
    int i;
    int remove_file;
    
    VALUE sArray = rb_ary_new();

    av_register_all();
    av_log_set_level(AV_LOG_PANIC);

    input = RSTRING_PTR(input_);
    output_prefix = RSTRING_PTR(output_prefix_);
    segment_duration = (FIX2LONG(duration_) - 1);
    
    //stdout = NULL;
    stderr = NULL;
    
    remove_filename = malloc(sizeof(char) * (strlen(output_prefix) + 15));
    if (!remove_filename) {
        fprintf(stderr, "Could not allocate space for remove filenames\n");
        exit(1);
    }
    
    output_filename = malloc(sizeof(char) * (strlen(output_prefix) + 15));
    if (!output_filename) {
        fprintf(stderr, "Could not allocate space for output filenames\n");
    }
    
        
    ifmt = av_find_input_format("mpegts");
    if (!ifmt) {
        fprintf(stderr, "Could not find MPEG-TS demuxer\n");
    }
    
    ret = av_open_input_file(&ic, input, ifmt, 0, NULL);
    if (ret != 0) {
        fprintf(stderr, "Could not open input file, make sure it is an mpegts file: %d %s\n", ret, input);
    }
    
    if (av_find_stream_info(ic) < 0) {
        fprintf(stderr, "Could not read stream information\n");
    }
    
    ofmt = guess_format("mpegts", NULL, NULL);
    if (!ofmt) {
        fprintf(stderr, "Could not find MPEG-TS muxer\n");
    }
    
    oc = avformat_alloc_context();
    if (!oc) {
        fprintf(stderr, "Could not allocated output context");
    }
    oc->oformat = ofmt;

    
    video_index = -1;
    audio_index = -1;
    
    for (i = 0; i < ic->nb_streams && (video_index < 0 || audio_index < 0); i++) {
        switch (ic->streams[i]->codec->codec_type) {
            case CODEC_TYPE_VIDEO:
                video_index = i;
                ic->streams[i]->discard = AVDISCARD_NONE;
                video_st = add_output_stream(oc, ic->streams[i]);
                break;
            case CODEC_TYPE_AUDIO:
                audio_index = i;
                ic->streams[i]->discard = AVDISCARD_NONE;
                audio_st = add_output_stream(oc, ic->streams[i]);
                break;
            default:
                ic->streams[i]->discard = AVDISCARD_ALL;
                break;
        }
    }
      
    if (av_set_parameters(oc, NULL) < 0) {
        fprintf(stderr, "Invalid output format parameters\n");
    }
    
    
    
    dump_format(oc, 0, output_prefix, 1);
    
    codec = avcodec_find_decoder(video_st->codec->codec_id);
    if (!codec) {
        fprintf(stderr, "Could not find video decoder, key frames will not be honored\n");
    }
    
    if (avcodec_open(video_st->codec, codec) < 0) {
        fprintf(stderr, "Could not open video decoder, key frames will not be honored\n");
    }
    
    snprintf(output_filename, strlen(output_prefix) + 15, "%s-%u.ts", output_prefix, output_index++);
    if (url_fopen(&oc->pb, output_filename, URL_WRONLY) < 0) {
        fprintf(stderr, "Could not open '%s'\n", output_filename);
    }
    
    if (av_write_header(oc)) {
        fprintf(stderr, "Could not write mpegts header to first output file\n");
    }

    
    //write_index = !write_index_file(index, tmp_index, segment_duration, output_prefix, http_prefix, first_segment, last_segment, 0, max_tsfiles);
    
    do {
        double segment_time;
        AVPacket packet;
        
        decode_done = av_read_frame(ic, &packet);
        if (decode_done < 0) {
            break;
        }
        
        if (av_dup_packet(&packet) < 0) {
            fprintf(stderr, "Could not duplicate packet");
            av_free_packet(&packet);
            break;
        }
        
        if (packet.stream_index == video_index && (packet.flags & PKT_FLAG_KEY)) {
            segment_time = (double)video_st->pts.val * video_st->time_base.num / video_st->time_base.den;
        }
        else if (video_index < 0) {
            segment_time = (double)audio_st->pts.val * audio_st->time_base.num / audio_st->time_base.den;
        }
        else {
            segment_time = prev_segment_time;
        }
        
        if (segment_time - prev_segment_time >= segment_duration) {
            put_flush_packet(oc->pb);
            url_fclose(oc->pb);
            
            if (max_tsfiles && (int)(last_segment - first_segment) >= max_tsfiles - 1) {
                remove_file = 1;
                first_segment++;
            }
            else {
                remove_file = 0;
            }
            
            Segment *segment = ALLOC(Segment);
            
            VALUE seg = Data_Wrap_Struct(rb_cAvSegment, 0, segment_free, segment);
            
            rb_iv_set(seg, "@index", INT2FIX(++last_segment));
            rb_iv_set(seg, "@duration",INT2FIX(segment_duration));
            rb_iv_set(seg, "@filename", rb_str_new2(output_filename));
            
            rb_ary_push(sArray, seg);
            
            if (remove_file) {
                snprintf(remove_filename, strlen(output_prefix) + 15, "%s-%u.ts", output_prefix, first_segment - 1);
                remove(remove_filename);
            }
            
            snprintf(output_filename, strlen(output_prefix) + 15, "%s-%u.ts", output_prefix, output_index++);
            if (url_fopen(&oc->pb, output_filename, URL_WRONLY) < 0) {
                fprintf(stderr, "Could not open '%s'\n", output_filename);
                break;
            }
            
            prev_segment_time = segment_time;
        }
        
        ret = av_interleaved_write_frame(oc, &packet);
        if (ret < 0) {
            fprintf(stderr, "Warning: Could not write frame of stream\n");
        }
        else if (ret > 0) {
            fprintf(stderr, "End of stream requested\n");
            av_free_packet(&packet);
            break;
        }
        
        av_free_packet(&packet);
    } while (!decode_done);
    
    av_write_trailer(oc);
    
    avcodec_close(video_st->codec);
    
    for(i = 0; i < oc->nb_streams; i++) {
        av_freep(&oc->streams[i]->codec);
        av_freep(&oc->streams[i]);
    }
    
    url_fclose(oc->pb);
    av_free(oc);
    
    if (max_tsfiles && (int)(last_segment - first_segment) >= max_tsfiles - 1) {
        remove_file = 1;
        first_segment++;
    }
    else {
        remove_file = 0;
    }
    
    //if (write_index) {
    //    write_index_file(index, tmp_index, segment_duration, output_prefix, http_prefix, first_segment, ++last_segment, 1, max_tsfiles);
   // }
    
    if (remove_file) {
        snprintf(remove_filename, strlen(output_prefix) + 15, "%s-%u.ts", output_prefix, first_segment - 1);
        remove(remove_filename);
    }
    
    return sArray;
}

void Init_segmenter_ext() {
    rb_mAvSegmenter = rb_define_module("Segmenter");
    rb_define_module_function(rb_mAvSegmenter, "segment", segmenter_segment, 3);
    
    rb_cAvSegment = rb_define_class_under(rb_mAvSegmenter, "Segment", rb_cObject);
    rb_define_singleton_method(rb_cAvSegment, "initialize", segment_init, 3);
    rb_define_alloc_func(rb_cAvSegment, segment_allocate);
    rb_define_attr(rb_cAvSegment, "duration", 1, 1);
    rb_define_attr(rb_cAvSegment, "index", 1, 1);
    rb_define_attr(rb_cAvSegment, "filename", 1, 1);
}                                       


