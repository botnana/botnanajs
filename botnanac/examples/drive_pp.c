#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "botnana.h"

// 處理主站傳回的資料
void on_message_cb (void * data, const char * src)
{
    printf("on_message:  %s\n", src);
}

// 處理 WebSocket on_open
void on_ws_open_cb (void * data, const char * src)
{
    int * open = (int *) data;
    *open = 1;
}

// 處理 WebSocket 連線異常
void on_ws_error_cb (void * data, const char * src)
{
    int * open = (int *) data;
    *open = 0;
    printf("WS client error: %s\n", src);
    exit(1);
}

void on_send_cb (void * data, const char * src)
{
    printf("on_send: %s\n", src);
}

void end_of_program (void * data, const char * src)
{
    if (strstr(src, "ok") != 0)
    {
        int * finished = (int *) data;
        *finished = 1;
    }
}

void deployed_cb (void * data, const char * src)
{
    int * ok = (int *) data;
    *ok = 1;
}


void real_position_cb (void * data, const char * src)
{
    int * pos = (int *) data;
    *pos = atoi(src);
}

void target_position_cb (void * data, const char * src)
{
    int * pos = (int *) data;
    *pos = atoi(src);
}

void log_cb (void * data, const char * src)
{
    printf("log|%s\n", src);
}

void error_cb (void * data, const char * src)
{
    printf("error|%s\n", src);
}


int main()
{
    int ws_open = 0;
    int is_finished = 0;
    int deployed_ok = 0;
    int real_position = 0;
    int target_position = 0;

    // connect to motion server
    struct Botnana * botnana = botnana_new("192.168.7.2");
    botnana_set_on_open_cb(botnana, (void *) & ws_open, on_ws_open_cb);
    botnana_set_on_error_cb(botnana, (void *) & ws_open, on_ws_error_cb);
    //botnana_set_on_message_cb(botnana, on_message_cb);
    //botnana_set_on_send_cb(botnana, on_send_cb);
    botnana_set_tag_cb(botnana, "end-of-program", 0, (void *) & is_finished, end_of_program);
    botnana_set_tag_cb(botnana, "deployed", 0, (void *) & deployed_ok, deployed_cb);
    botnana_set_tag_cb(botnana, "real_position.1.1", 0,  (void *) & real_position, real_position_cb);
    botnana_set_tag_cb(botnana, "target_position.1.1", 0,  (void *) & target_position, target_position_cb);
    botnana_set_tag_cb(botnana, "log", 0, NULL, log_cb);
    botnana_set_tag_cb(botnana, "error", 0, NULL ,error_cb);
    // new program
    struct Program * pm = program_new("drive-pp");

    botnana_connect(botnana);
    while (ws_open == 0)
    {
        sleep(1);
    }

    // reset drive 1 fault
    program_line(pm, "1 1 reset-fault");

    // wait drive 1 to no fault
    program_line(pm, "1 1 until-no-fault");

    // drive 1 servo on
    program_line(pm, "1 1 drive-on");

    // wait drive 1 to servo on
    program_line(pm, "1 1 until-drive-on");

    // wait 1000 ms
    program_line(pm, "1000 ms");

    // set drive 1 to HM mode
    program_line(pm, "hm 1 1 op-mode!");

    // set homing method to 33
    program_line(pm," 33 1 1 homing-method!");

    // wait HM request finished
    program_line(pm, "until-no-requests");

    // start drive 1 homing
    program_line(pm, "1 1 go");

    // wait drive 1 homing finished
    program_line(pm, "1 1 until-target-reached");
    program_line(pm, ".\" log|homed\" cr ");

    // set drive to PP mode
    program_line(pm, "pp 1 1 op-mode!");
    program_line(pm, "1000 1 1 profile-v!");

    //wait PP request finished
    program_line(pm, "until-no-requests");

    //set drive 1 target position to 10000 pulse count
    program_line(pm, "10000 1  1 target-p!");

    // start drive 1 moving
    program_line(pm, "1 1 go");

    // wait drive target reached
    program_line(pm, "1 1 until-target-reached");

    // deploy program to motion server
    botnana_abort_program(botnana);
    script_evaluate(botnana, "-work marker -work");
    program_deploy(botnana,pm);

    while (deployed_ok == 0)
    {
        sleep(1);
    }

    // run program
    program_run(botnana, pm);

    while (1)
    {
        script_evaluate(botnana, "1 .slave");
        printf("target position: %d, real position: %d, is_finished: %d\n",target_position, real_position, is_finished);
        sleep(1);
    }
    return 0;

}

