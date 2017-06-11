#include <unistd.h>
#include <stdlib.h>
extern "C" {
#include <soundpipe.h>
#include <sporth.h>
#include "spigot.h"
spigot_graphics *global_gfx;
}
#include "RtAudio.h"

static int add_function(plumber_data *pd, void *ud)

{
    plumber_ftmap_add_function(pd, "spigot", sporth_spigot, NULL);
    return PLUMBER_OK;
}

static int callme( void * outputBuffer, void * inputBuffer, unsigned int numFrames,
            double streamTime, RtAudioStreamStatus status, void * data )
{
    SPFLOAT *output = (SPFLOAT *) outputBuffer;    
    unsigned int i;
    
    plumber_data *pd = (plumber_data *)data;

    if(pd->recompile) {
        plumber_recompile_v2(pd, NULL, add_function);
        pd->recompile = 0;
    }

    for(i = 0; i < numFrames * 2; i+= 2) {
        plumber_compute(pd, PLUMBER_COMPUTE);
        output[i] = sporth_stack_pop_float(&pd->sporth.stack);
        output[i + 1] = output[i];
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
    sp->sr = info.preferredSampleRate;
    plumber_register(&pd);
    plumber_init(&pd);

    add_function(&pd, NULL);
    plumber_open_file(&pd, argv[1]);
    
    pd.sp = sp;
    global_gfx = spigot_gfx_new(3);

    spigot_set_recompile(global_gfx, &pd.recompile);
    if(plumber_parse(&pd) != PLUMBER_OK) {
        plumber_print(&pd, "Error.\n");
        goto clean;
    } else {
        plumber_compute(&pd, PLUMBER_INIT); 
        audio.startStream();
    }

    if(!spigot_loaded(global_gfx)) {
        plumber_print(&pd, "Spigot state not loaded!\n");
        goto clean;
    }

    if(spigot_is_it_happening(global_gfx)) {
        spigot_gfx_init(global_gfx);
        spigot_graphics_loop(global_gfx);
    } else {
        fgetc(stdin);
    }

clean:
    audio.stopStream();
    if(spigot_is_it_happening(global_gfx)) {
        spigot_stop(global_gfx);
    }
    spigot_gfx_free(global_gfx);
    plumber_close_file(&pd);
    plumber_clean(&pd);
    sp_destroy(&sp);

    return 0;
}
