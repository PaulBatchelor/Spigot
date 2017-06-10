#include <unistd.h>
#include <stdlib.h>
extern "C" {
#include <soundpipe.h>
#include <sporth.h>
#include "spigot.h"
}
#include "RtAudio.h"

spigot_graphics *global_gfx;

static int callme( void * outputBuffer, void * inputBuffer, unsigned int numFrames,
            double streamTime, RtAudioStreamStatus status, void * data )
{
    SPFLOAT *output = (SPFLOAT *) outputBuffer;    
    unsigned int i;
    
    plumber_data *pd = (plumber_data *)data;

    for(i = 0; i < numFrames * 2; i+= 2) {
        plumber_compute(pd, PLUMBER_COMPUTE);
        output[i] = sporth_stack_pop_float(&pd->sporth.stack);
        output[i + 1] = sporth_stack_pop_float(&pd->sporth.stack);
    }
    return 0;
}

int main(int argc, char *argv[])
{
    sp_data *sp;
    plumber_data pd;
    RtAudio audio;
    RtAudio::DeviceInfo info; 
    RtAudio::StreamParameters iParams, oParams;
    unsigned int buffer_frames = 1024;
    info = audio.getDeviceInfo(audio.getDefaultOutputDevice());
    iParams.deviceId = audio.getDefaultInputDevice();
    iParams.nChannels = 0;
    iParams.firstChannel = 0;
    oParams.deviceId = audio.getDefaultOutputDevice();
    oParams.nChannels = 2;
    oParams.firstChannel = 0;
    
    RtAudio::StreamOptions options;
    
    if(argc == 1) {
        fprintf(stderr, "Usage %s file.sp\n", argv[0]);
        exit(1);
    }
   
    audio.openStream( &oParams, NULL, 
            RTAUDIO_FLOAT32, info.preferredSampleRate, 
            &buffer_frames, &callme, &pd, &options);
    audio.showWarnings( true );

    sp_create(&sp);
    plumber_register(&pd);
    plumber_init(&pd);

    plumber_open_file(&pd, argv[1]);
    
    pd.sp = sp;
    if(plumber_parse(&pd) != PLUMBER_OK) {
        plumber_print(&pd, "Error.\n");
        goto clean;
    } else {
        plumber_compute(&pd, PLUMBER_INIT); 
        audio.startStream();
    }
   
    sleep(10);
clean:
    plumber_close_file(&pd);
    plumber_clean(&pd);
    sp_destroy(&sp);

    return 0;
}
