\
\ Queue
\ +-------+------+--------+--------+--------+-----+
\ | front | back | cell 0 | cell 1 | cell 2 | ... |
\ +-------+------+--------+--------+--------+-----+

: create-queue ( capacity -- )
    create 0 , 0 , cells allot ;
\    does> ( n -- a ) swap 2 +  cells + ;
100 constant #position        \ 陣列大小為 100
-1 constant serveroff         \ 將關閉指令命名為serveroff並賦值-1
0 constant serveron           \ 將開啟指令命名為serveron並賦值0
-2 constant inpos             \ 將等待抵達指令命名為inpos並賦值-2
-3 constant acc               \ 將指定加速度命令命名為acc並賦值-3
-4 constant dec               \ 將指定減速度命令命名為dec並賦值-4
-5 constant quickstop		  \ 將停止馬達命令命名為quickstop並賦值-5
-6 constant slowstop
\ -6 constant haltslow		  \ 將減速停止命令命名為haltslow -6
\ -7 constant haltquick		  \ 將直接停止命令命名為haltquick並賦值-7
\ -8 constant haltend			  \ 將停止暫停命令命名為haltend並賦值-8
-9 constant pp-v			  \ 將指定pp模式速度命令命名為pp-v並賦值-9
-10 constant get-sdo
-11 constant set-sdo
-12 constant station-no-set
#position create-queue pp[]   \ 定義陣列名稱為 position[]
#position create-queue pv[]
variable dequeue_front 0 dequeue_front !
variable dequeue_back 0 dequeue_back !
variable slave 0 slave !
variable num 0 num !
\ variable front#   0 front# ! \ queue 前端的索引
\ variable back#    0 back# !  \ queue 尾端的索引

\ 取得queue前端及尾端的索引
: front@ ( queue -- front )  @ ;
: back@ ( queue -- back )   cell+ @ ;

\ 判斷 queue 是否是空的
\ : empty? ( -- f )   front# @  back# @  = ;
: empty? ( queue -- f ) dup front@  swap back@ = ;

\ 判斷 queue 是否是滿的
\ : full? ( -- f )   back# @  1 +  #position mod  front# @  = ;
: full? ( queue -- f ) dup back@ 1 + #position mod swap front@ = ;


\ : queue ( position -- )
\    full? not if
\        back# @  position[] !
\        back# @  1 +  #position mod  back# !
\    else
\        ." log|full!" cr
\    then ;
\ 將 position 放進 queue 中。如果無法放進去，傳一訊息給上位控制器。
: queue ( queue position -- )
    swap dup 
    full? not if
        dup -rot dup back@ 2 + cells + !
        dup back@ 1 + #position mod swap cell+ ! 
    else
        ." log|full!" cr
    then ;

\ 一次將兩個元素放進 queue 中。
: 2queue ( queue mode position -- )
    -rot 2dup queue
    drop swap queue ;

\ 一次將四個元素放進 queue 中。
: 4queue ( queue mode position slave n -- )
    pp[] -rot 2queue 2queue ;

\ 從 queue 中取出一個值。如果 queue 是空的，回傳的 f 值為 0 (false)，否則回傳那個值以及 -1 (true)。
\ : dequeue ( -- position true | false)
\    empty? not if
\        front# @  position[] @
\        front# @  1 +  #position mod  front# !
\        true
\    else
\        false
\    then ;

\ 從 queue 中取出一個值。如果 queue 是空的，回傳的 f 值為 0 (false)，否則回傳那個值以及 -1 (true)。
\ 並且將front、back以外的資料都往前放一個位置
: dequeue ( -- position true | false ) 
    dup front@ 2 + dequeue_front ! 
    dup back@ 2 + dequeue_back ! 
    dup 
    empty? not if 
        dup 
        dup front@  2 + cells + @ true rot ( ans true p ) 
        begin dequeue_front @ dequeue_back @ < 
        while dup dup dequeue_front @ 1 + cells + @ swap dequeue_front @ cells + ! 
        dequeue_front @ 1 + dequeue_front ! 
        repeat 
        dup cell+ @ 1 - swap cell+ ! 
    else
        drop false
    then ;
\ 清空queue，作法為將queue末端索引改為0即可
: clear ( position -- )
    0 swap cell+ ! ;

\
\ NC task
\
: drive-pp
    1 1 reset-fault
    1 1 until-no-fault
    1 1 drive-on
    1 1 until-drive-on
    1000 ms
    ." log|homed" cr
    pp  1 1 op-mode!
    1000  1 1 profile-v!
    until-no-requests
    begin
        begin
            pp[] dequeue not              \ 從 positions 中拿出一個位置，
        while                        \ 如果positions 是空的
            pause                    \ 把 CPU 使用權交給其他 tasks
                                     \ 注意這個 pause 是必須的。如果不把 CPU 使用
                                     \ 權交出去，那麼，另一個 task 就不能執行 queue
                                     \ 放進新的位置，於是這兒就形成一個忙碌迴圈，
                                     \ 又因為這個指令是在最高優先的 real-time 環境下
                                     \ 執行，整個控制器就會當在這兒。直到控制器更高優先
                                     \ 權的 watch dog thread 超時強迫 tasks 停止。 
        repeat
        ( position )  1 1 target-p!  \ 設定新的目標位置竹為從 positions 得到的位置
        1 1 go                       \ 移動至新的位置
        1 1 until-target-reached     \ 等待到達目標位置
    again
;

: drive-pv
    1 1 reset-fault
    1 1 until-no-fault
    1 1 drive-on
    1 1 until-drive-on
    1000 ms 
    
    ." log|homed" cr
    pv  1 1 op-mode!
    -10000  1 1 target-v!
    until-no-requests
    begin
        begin
           pv[] dequeue not              \ 從 positions 中拿出一個位置，
        while                        \ 如果positions 是空的
            pause                    \ 把 CPU 使用權交給其他 tasks
                                     \ 注意這個 pause 是必須的。如果不把 CPU 使用
                                     \ 權交出去，那麼，另一個 task 就不能執行 queue
                                     \ 放進新的位置，於是這兒就形成一個忙碌迴圈，
                                     \ 又因為這個指令是在最高優先的 real-time 環境下
                                     \ 執行，整個控制器就會當在這兒。直到控制器更高優先
                                     \ 權的 watch dog thread 超時強迫 tasks 停止。 
        repeat
        1 1 target-v!
     again
;


\ 2drive
\ 此指令為固定從站編號1第1管道的馬達的情況下，可以自由指定此馬達為pp/pv模式
\ 指令為 pp[] mode position/velocity 2queue
\ 例：想指定為pp模式，位置為10000，輸入 pp[] pp 10000 2queue
\ 例：想指定為pv模式，速度為10000，輸入 pp[] pv 10000 2queue
\ 輸入後，queue的內容為： | mode | position/velocity |
: 2drive
    1 1 reset-fault
    1 1 until-no-fault
    1 1 drive-on
    1 1 until-drive-on
    1000 ms 
    ." log|homed" cr

    begin 
        begin
            pp[] dequeue not                   \ 從queue裡取出mode，若queue為空則繼續重新dequeue
        while
            pause 
        repeat
        pp[] dequeue drop swap                 \ 從queue裡取出position/velocity
        case                                   
            pp of                              
                pp  1 1 op-mode!
                100000  1 1 profile-v!
                until-no-requests
                1 1 target-p!
                1 1 go
                1 1 until-target-reached
            endof
            pv of
                pv  1 1 op-mode!
                1 1 target-v!
                until-no-requests
            endof
            drop 
        endcase
    again
;


\ 4drive
\ 此指令可以自由指定從站編號以及馬達管道編號的情況下，自由指定此馬達為pp/pv模式
\ 指令為 pp[] mode 0/position/velocity/homing-method ch slave 4queue
\ mode有五種，-1 0 pp pv hm
\ 例：關第1從站第1馬達，輸入：pp[] -1 0 1 1 4queue
\ 例：啟動第1從站第1馬達，輸入：pp[] 0 0 1 1 4queue
\ 例：想指定第2從站第1馬達為pp模式，位置為10000，輸入 pp[] pp 10000 1 2 4queue
\ 例：想指定第1從站第2馬達為pv模式，速度為10000，輸入 pp[] pv 10000 2 1 4queue
\ 例：想指定第1從站第2馬達為hm模式，方式為 33，輸入 pp[] hm 33 2 1 4queue
\ 輸入後，queue的內容為： | ch | slave | mode | position/velocity/homing-method |
: 4drive
    ." log|homed" cr        
    begin 
        
        begin
            pp[] dequeue not                        \ 從queue裡取出ch，若queue為空則繼續重新dequeue
        while
            pause 
        repeat
        pp[] dequeue drop                           \ 從queue裡取出slave
        slave !                                     
        num !
        

        pp[] dequeue drop                           \ 從queue裡取出mode
        pp[] dequeue drop swap                      \ 從queue裡取出position

        case 
            serveroff of
                num @ slave @ reset-fault           \ 清除馬達的異警
                num @ slave @ until-no-fault        \ 等待直到馬達的異警被清除
                num @ slave @ drive-off             \ 關閉馬達
                100 ms 
            endof
            serveron of
                num @ slave @ reset-fault
                num @ slave @ until-no-fault
                num @ slave @ drive-on              \ 將馬達致能 (servo on, operation enabled)
                num @ slave @ until-drive-on        \ 等待從站一的第一顆馬達完成致能
                1000 ms 
            endof
            inpos of
                num @ slave @ until-target-reached  \ 等待馬達到達指定位置
            endof
            acc of
                num @ slave @ profile-a1!
            endof
            dec of
                num @ slave @ profile-a2!
            endof
            pp-v of
                num @ slave @ profile-v!    \ 設馬達的速度 (profile velocity) 為 100000
                until-no-requests                   \ 等待之前的 SDO 設定完成
            endof
            pp of
                pp  num @ slave @ op-mode! 
                until-no-requests 
                num @ slave @ target-p!             \ 設定馬達的目標位置 (target position)
                num @ slave @ go                    \ 執行
            endof
            pv of
                pv  num @ slave @ op-mode!
                num @ slave @ target-v!
                until-no-requests
            endof
            hm of
                hm  num @ slave @ op-mode!          \ 設馬達的模式為回原點 (hm mode)
                num @ slave @ homing-method!     \ 設定為原典的方式
                until-no-requests
                num @ slave @ go
            endof
            quickstop of
                -1 0 $605A num @ sdo-download-i16
                until-no-requests
                num @ slave @ drive-stop
                pp[] clear 
                num @ slave @ reset-fault
                num @ slave @ until-no-fault
            endof
            slowstop of
                0 $6085 num @ sdo-download-i32
                6 0 $605A num @ sdo-download-i16
                until-no-requests
                num @ slave @ drive-stop
                pp[] clear 
                num @ slave @ reset-fault
                num @ slave @ until-no-fault
            endof
            get-sdo ( subindex index position ) ( pp[] subindex index position 4queue ) of
                num @ slave @ sdo-upload-i32 begin slave @ sdo-busy? while pause repeat slave @ .sdo until-no-requests
            endof
            set-sdo ( data subindex index position ) ( pp[] data index+subindex position 4queue ) of
                num @ 16 mod num @ 16 / slave @ sdo-download-i32 until-no-requests 
            endof
            station-no-set of
                slave @ ec-alias!
            endof
            drop 
        endcase
    again
;
