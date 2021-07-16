#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "botnana.h"
char pds_state[100] = "unknown";
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

void digital_inputs_cb (void * data, const char * src)
{
    int * pos = (int *) data;
    *pos = atoi(src);
}

void real_velocity_cb (void * data, const char * src)
{
    int * pos = (int *) data;
    *pos = atoi(src);
}

void demand_velocity_cb (void * data, const char * src)
{
    int * pos = (int *) data;
    *pos = atoi(src);
}

void demand_position_cb (void * data, const char * src)
{
    int * pos = (int *) data;
    *pos = atoi(src);
}

void demand_torque_cb (void * data, const char * src)
{
    int * pos = (int *) data;
    *pos = atoi(src);
}

void pds_state_cb (void * data, const char * src)
{
    int i = 0;
    while(src[i] != '\0'){
        pds_state[i] = src[i];
        i++;
    }
    pds_state[i] = '\0';
}

void sdo_index_cb (void * data, const char * src)
{
    int * pos = (int *) data;
    *pos = (int)strtol(src, NULL, 0);
    printf("\n sdo_index: 0x%08x | ", (int)strtol(src, NULL, 0));
}

void sdo_subindex_cb (void * data, const char * src)
{
    int * pos = (int *) data;
    *pos = atoi(src);
    printf("sdo_subindex: 0x%08x | ", (int)strtol(src, NULL, 0));
}

void sdo_data_cb (void * data, const char * src)
{
    int * pos = (int *) data;
    *pos = atoi(src);
    printf("sdo_data: 0x%08x\n\n", (int)strtol(src, NULL, 0));
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
    int digital_inputs = 0;
    int real_velocity = 0;
    int demand_velocity = 0;
    int demand_position = 0;
    int demand_torque = 0;
    
    int sdo_index = 0;
    int sdo_subindex = 0;
    int sdo_data = 0;


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
    botnana_set_tag_cb(botnana, "digital_inputs.1.1", 0,  (void *) & digital_inputs, digital_inputs_cb);
    botnana_set_tag_cb(botnana, "real_velocity.1.1", 0, (void *) & real_velocity, real_velocity_cb);
    botnana_set_tag_cb(botnana, "demand_velocity.1.1", 0, (void *) & demand_velocity, demand_velocity_cb);
    botnana_set_tag_cb(botnana, "demand_position.1.1", 0, (void *) & demand_position, demand_position_cb);
    botnana_set_tag_cb(botnana, "demand_torque.1.1", 0, (void *) & demand_torque, demand_torque_cb);
    botnana_set_tag_cb(botnana, "pds_state.1.1", 0, (void *) & pds_state, pds_state_cb);
    botnana_set_tag_cb(botnana, "sdo_index.1", 0, (void *) & sdo_index, sdo_index_cb);
    botnana_set_tag_cb(botnana, "sdo_subindex.1", 0, (void *) & sdo_subindex, sdo_subindex_cb);
    botnana_set_tag_cb(botnana, "sdo_data_hex.1", 0, (void *) & sdo_data, sdo_data_cb);
    botnana_set_tag_cb(botnana, "log", 0, NULL, log_cb);
    botnana_set_tag_cb(botnana, "error", 0, NULL ,error_cb);
    

    botnana_connect(botnana);
    while (ws_open == 0)
    {
        sleep(1);
    }

    // Create a new program with name 'main', using library from file ./drive-pp.fs.
    struct Program * pm = program_new_with_file("main", "./drive-pp.fs");
    // The main programe will execute drive-pp.
    program_line(pm, "4drive");

    // Deploy program to motion server
    botnana_abort_program(botnana);
    script_evaluate(botnana, "-work marker -work");
    program_deploy(botnana, pm);

    while (deployed_ok == 0)
    {
        sleep(1);
    }

    // Run program
    program_run(botnana, pm);

    // 指令為 pp[] mode 0/position/velocity/homing-method ch slave 4queue
    // mode有五種，-1 0 pp pv hm
    // 例：關第1從站第1馬達，輸入：pp[] -1 0 1 1 4queue
    // 例：啟動第1從站第1馬達，輸入：pp[] 0 0 1 1 4queue
    // 例：想指定第2從站第1馬達為pp模式，位置為10000，輸入 pp[] pp 10000 1 2 4queue
    // 例：想指定第1從站第2馬達為pv模式，速度為10000，輸入 pp[] pv 10000 2 1 4queue
    // 例：想指定第1從站第2馬達為hm模式，方式為 33，輸入 pp[] hm 33 2 1 4queue
    // 輸入後，queue的內容為： | ch | slave | mode | 0/position/velocity/homing-method |
    // mode有三種，pp pv 0
    // 例：啟動第1從站第1管道馬達，輸入：pp[] 0 0 1 1 4queue
    // 例：想指定第2從站第1管道馬達為pp模式，位置為10000，輸入 pp[] pp 10000 1 2 4queue
    // 例：想指定第1從站第2管道馬達為pv模式，速度為10000，輸入 pp[] pv 10000 2 1 4queue

    // 清除第一從站上的第二馬達的異警後，servo on，清除第二從站上第一馬達的異警後 servo on。 
    // 其中，pp[] 是 queue 的名字，未來也許該改名。
    //script_evaluate(botnana, "pp[] serveron 0 1 1 4queue   pp[] serveron 0 1 2 4queue");
    // 第一從站上的第一馬達以 hm 方式模式 33 回原點。第二從站上的第一馬達以 hm 方式模式 33 回原點。
    // 這裡的hm不需等待其他的馬達完成指令，故兩顆馬達可同時回原點。
    //script_evaluate(botnana, "pp[] hm 33 1 1 4queue  pp[] hm 33 1 2 4queue");
    //script_evaluate(botnana, "pp[] inpos 0 1 1 4queue pp[] inpos 0 1 2 4queue");
    // 第一從站上的第一馬達以 pp 方式走到 10000 的位置。第二從站上的第一馬達以 pp 方式走到 20000 的位置。
    // 這裡的pp不需等待其他的馬達完成指令，故兩顆馬達可同時轉動。
    //script_evaluate(botnana, "pp[] pp 10000 1 1 4queue  pp[] pp 20000 1 2 4queue");
    //script_evaluate(botnana, "pp[] inpos 0 1 1 4queue pp[] inpos 0 1 2 4queue");
    // 第一從站上的第一馬達以 pv 模式走，目標速度為 30000。第二從站上的第一馬達以 pv 模式走，目標速度為 40000 的位置。
    //script_evaluate(botnana, "pp[] pv 30000 1 1 4queue  pp[] pv 40000 1 2 4queue");
    // 清除第一從站上的第二馬達的異警後，servo off，清除第二從站上第一馬達的異警後 servo off。 
    //script_evaluate(botnana, "pp[] serveroff 0 1 1 4queue   pp[] serveroff 0 1 2 4queue");

    subscribe_ec_slave(botnana,0,1);

    while (1)
    {
        //script_evaluate(botnana, "1 .slave");
        printf("target position: %d, real position: %d, is_finished: %d, digital_inputs: %d, real_velocity: %d\n",
                target_position, real_position, is_finished, digital_inputs, real_velocity);
        printf("demand_velocity: %d, demand_position: %d, demand_torque: %d, pds_state: %s\n", 
                demand_velocity, demand_position, demand_torque, pds_state);
        sleep(1);
    }
    return 0;

}