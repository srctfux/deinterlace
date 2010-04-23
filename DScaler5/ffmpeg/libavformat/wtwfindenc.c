/*
 * wtw finder
 * Copyright (c) 2010 John Adcock
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "avformat.h"

typedef struct wtwfind_context {
    uint64_t frame_count;
    uint64_t max_histogram[3][256];
    uint64_t min_histogram[3][256];
} WtwFindContext;


static int wtwfind_write_header(AVFormatContext *s)
{
    WtwFindContext *wtw = s->priv_data;
    char buf[256];

    if (s->streams[0]->codec->pix_fmt == PIX_FMT_RGB24)
    {
        snprintf(buf, sizeof(buf), "Counter,NumRPixelsOverRange,NumGPixelsOverRange,NumBPixelsOverRange,PeakR,PeakG,PeakB,NumRPixelsUnderRange,NumGPixelsUnderRange,NumBPixelsUnderRange,MinR,MinG,MinB\r\n");
    }
    else if(s->streams[0]->codec->pix_fmt == PIX_FMT_YUV420P)
    {
        snprintf(buf, sizeof(buf), "Counter,NumYPixelsOverRange,NumCrPixelsOverRange,NumCbPixelsOverRange,PeakY,PeakCr,PeakCb,NumYPixelsUnderRange,NumCrPixelsUnderRange,NumCbPixelsUnderRange,MinY,MinCr,MinCb\r\n");
    }
    else
    {
        av_log(s, AV_LOG_ERROR, "Only PIX_FMT_RGBA and PIX_FMT_YUV420 are supported\n");
        return AVERROR_INVALIDDATA;
    }

    put_buffer(s->pb, buf, strlen(buf));
    put_flush_packet(s->pb);

    wtw->frame_count = 0;
    for(int i = 0; i < 3; ++i)
        for(int j =0; j < 256; ++j)
        {
            wtw->max_histogram[i][j] = 0;
            wtw->min_histogram[i][j] = 0;
        }
    return 0;
}

static int wtwfind_write_packet_rgb24(struct AVFormatContext *s, AVPacket *pkt)
{
    AVCodecContext* codecContext = s->streams[0]->codec;
    WtwFindContext *wtw = s->priv_data;
    uint32_t counthi[3] = {0,0,0};
    uint32_t countlo[3] = {0,0,0};
    uint8_t peak[3] = {0,0,0};
    uint8_t valley[3] = {255,255,255};
    char buf[256];

    uint32_t offsety = codecContext->height / 20;
    uint32_t offsetx = codecContext->width / 20;

    for(uint32_t i = offsety; i < codecContext->height - offsety; ++i)
    {
        uint8_t* pBuff = (uint8_t*)pkt->data + i * codecContext->width * 3;
        for(uint32_t j = offsetx; j < codecContext->width -offsetx; ++j)
        {
            for(int k = 0; k < 3; ++k)
            {
                uint8_t colour = *pBuff++;
                if(colour > 235)
                {
                    ++counthi[k];
                }
                if(colour < 16)
                {
                    ++countlo[k];
                }
                if(colour > peak[k])
                {
                    peak[k] = colour;
                }
                if(colour < valley[k])
                {
                    valley[k] = colour;
                }
            }
        }
    }

    for(int i = 0; i < 3; ++i)
    {
        ++(wtw->max_histogram[i][peak[i]]);
        ++(wtw->min_histogram[i][valley[i]]);
    }

    snprintf(buf, sizeof(buf), "%"PRId64",%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\r\n", ++wtw->frame_count, counthi[0], counthi[1], counthi[2], peak[0], peak[1], peak[2], countlo[0], countlo[1], countlo[2], valley[0], valley[1], valley[2]);
    put_buffer(s->pb, buf, strlen(buf));
    put_flush_packet(s->pb);
    return 0;
}

static int wtwfind_write_packet_yuv(struct AVFormatContext *s, AVPacket *pkt)
{
    AVCodecContext* codecContext = s->streams[0]->codec;
    WtwFindContext *wtw = s->priv_data;
    uint32_t counthi[3] = {0,0,0};
    uint32_t countlo[3] = {0,0,0};
    uint8_t peak[3] = {0,0,0};
    uint8_t valley[3] = {255,255,255};
    char buf[256];
    static uint64_t counter = 0;

    uint32_t offsety = codecContext->height / 20;
    uint32_t offsetx = codecContext->width / 20;

    uint8_t* pBuff = (uint8_t*)pkt->data;
    for(uint32_t i = offsety; i < codecContext->height - offsety; ++i)
    {
        for(uint32_t j = offsetx; j < codecContext->width - offsetx; ++j)
        {
            uint8_t colour = *pBuff++;
            if(colour > 235)
            {
                ++counthi[0];
            }
            if(colour < 16)
            {
                ++countlo[0];
            }
            if(colour > peak[0])
            {
                peak[0] = colour;
            }
            if(colour < valley[0])
            {
                valley[0] = colour;
            }
        }
    }
    for(uint32_t i = offsety / 2; i < (codecContext->height - offsety) / 2; ++i)
    {
        for(uint32_t j = offsetx / 2; j < (codecContext->width - offsetx)/ 2; ++j)
        {
            uint8_t colour = *pBuff++;
            if(colour > 240)
            {
                ++counthi[1];
            }
            if(colour < 16)
            {
                ++countlo[1];
            }
            if(colour > peak[1])
            {
                peak[1] = colour;
            }
            if(colour < valley[1])
            {
                valley[1] = colour;
            }
        }
    }
    for(uint32_t i = offsety / 2; i < (codecContext->height - offsety) / 2; ++i)
    {
        for(uint32_t j = offsetx / 2; j < (codecContext->width - offsetx) / 2; ++j)
        {
            uint8_t colour = *pBuff++;
            if(colour > 235)
            {
                ++counthi[2];
            }
            if(colour < 16)
            {
                ++countlo[2];
            }
            if(colour > peak[2])
            {
                peak[2] = colour;
            }
            if(colour < valley[2])
            {
                valley[2] = colour;
            }
        }
    }

    for(int i = 0; i < 3; ++i)
    {
        ++(wtw->max_histogram[i][peak[i]]);
        ++(wtw->min_histogram[i][valley[i]]);
    }

    snprintf(buf, sizeof(buf), "%"PRId64",%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\r\n", ++counter, counthi[0], counthi[1], counthi[2], peak[0], peak[1], peak[2], countlo[0], countlo[1], countlo[2], valley[0], valley[1], valley[2]);
    put_buffer(s->pb, buf, strlen(buf));
    put_flush_packet(s->pb);
    return 0;
}

static int wtwfind_write_packet(struct AVFormatContext *s, AVPacket *pkt)
{
    if (s->streams[0]->codec->pix_fmt == PIX_FMT_RGB24)
    {
        return wtwfind_write_packet_rgb24(s, pkt);
    }
    else if(s->streams[0]->codec->pix_fmt == PIX_FMT_YUV420P)
    {
        return wtwfind_write_packet_yuv(s, pkt);
    }
    return -1;
}

static int wtwfind_write_trailer(struct AVFormatContext *s)
{
    WtwFindContext *wtw = s->priv_data;
    char buf[256];
    if (s->streams[0]->codec->pix_fmt == PIX_FMT_RGB24)
    {
        snprintf(buf, sizeof(buf), "Value,RedFramesPeak,GreenFramesPeak,BlueFramesPeak,RedFramesMin,GreenFramesMin,BlueFramesMin\r\n");
    }
    else if(s->streams[0]->codec->pix_fmt == PIX_FMT_YUV420P)
    {
        snprintf(buf, sizeof(buf), "Value,YFramesPeak,CrFramesPeak,CbFramesPeak,YFramesMin,CrFramesMin,CbFramesMin\r\n");
    }
    put_buffer(s->pb, buf, strlen(buf));
    put_flush_packet(s->pb);
    for(int j =0; j < 256; ++j)
    {
        snprintf(buf, sizeof(buf), "%d,%"PRId64",%"PRId64",%"PRId64",%"PRId64",%"PRId64",%"PRId64"\r\n", j, wtw->max_histogram[0][j], wtw->max_histogram[1][j], wtw->max_histogram[2][j], wtw->min_histogram[0][j], wtw->min_histogram[1][j], wtw->min_histogram[2][j]);
        put_buffer(s->pb, buf, strlen(buf));
        put_flush_packet(s->pb);
    }
    return 0;
}

AVOutputFormat wtwfind_muxer = {
    "wtwfind",
    NULL_IF_CONFIG_SMALL("wtwfind testing format"),
    NULL,
    "",
    sizeof(WtwFindContext),
    CODEC_ID_NONE,
    CODEC_ID_RAWVIDEO,
    wtwfind_write_header,
    wtwfind_write_packet,
    wtwfind_write_trailer,
};
