//
//  iosegmenter.c
//  iosegmenter
//
//  Created by Emil Palm on 4/5/11.
//  Copyright 2011 none. All rights reserved.
//

#include "ruby.h"

static VALUE rb_mIos;
static VALUE rb_cIosSegmenter;

void Init_av_segmenter() {
    rb_mIos = rb_define_module("Ios");
    rb_cIosSegmenter = rb_define_class_under(rb_mIos, "Segmenter", rb_cObject);
    rb_define_attr(rb_cIosSegmenter, "input", 1, 1);
    rb_define_attr(rb_cIosSegmenter, "segment_duration", 1, 1);
    rb_define_attr(rb_cIosSegmenter, "output_prefix", 1, 1);
}                                       

/*                                          
void
Init_FFMPEGFormat() {
    //fprintf(stderr, "init FFMPEG Format\n");
    rb_cFFMPEGFormat = rb_define_class("IosSegmeter", rb_cObject);
    
    //fprintf(stderr, "init FFMPEG Format allocation\n");
    rb_define_alloc_func(rb_cFFMPEGFormat, alloc_format);
    
    //fprintf(stderr, "init FFMPEG Format initialize\n");
    rb_define_method(rb_cFFMPEGFormat, "initialize", init_format, 0);
    rb_define_method(rb_cFFMPEGFormat, "filename", format_filename, 0);
    rb_define_method(rb_cFFMPEGFormat, "bit_rate", format_bit_rate, 0);
    rb_define_method(rb_cFFMPEGFormat, "duration", format_duration, 0);
    rb_define_method(rb_cFFMPEGFormat, "human_duration", format_duration_human, 0);
    
    rb_define_method(rb_cFFMPEGFormat, "streams", format_streams, 0);
    rb_define_method(rb_cFFMPEGFormat, "video_stream_count", format_video_stream_count, 0);
    rb_define_method(rb_cFFMPEGFormat, "audio_stream_count", format_audio_stream_count, 0);
    rb_define_method(rb_cFFMPEGFormat, "has_stream_with_codec_type?", format_has_stream_with_codec_type, 1);
    rb_define_method(rb_cFFMPEGFormat, "has_video?", format_has_video, 0);
    rb_define_method(rb_cFFMPEGFormat, "has_audio?", format_has_audio, 0);
}*/
