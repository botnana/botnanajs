# 馬達控制

本章節將介紹如何使用4drive配合4queue指令控制馬達。

首先介紹queue的操作指令，都是以下列指令進行操作。

```
queue_name  operation  parameter  channel  slave_num  4queue
```
queue_name：queue的名字。

operation：想對馬達操作的指令，總共有：啟動(serveron)、關閉(serveroff)、pp模式(pp)、pv模式(pv)、回歸原點模式(hm)、等待到達(inpos)、指定加速度(acc)、指定減速度(dec)、直接停止馬達(quickstop)、減速停止馬達(slowstop)、設定從站alias(station-no-set)。

parameter：配合operation指定的參數，適用於pp、pv，若是其他的operation則此欄位隨意填入數字即可。

channel：該馬達的管道編號。

slave_num：連結該馬達的從站編號。

4queue：此為固定的指令，表示將前四項參數放入queue中。

* 指令範例(以下範例都以編號1從站的編號1馬達進行示範，並且queue_name都為"pp[]")

* 開啟馬達
```
pp[] serveron 0 1 1 4queue
\ 此處的0可為任意數
```
* 關閉馬達
```
pp[] serveroff 0 1 1 4queuue
\ 此處的0可為任意數
```
* 回歸原點模式，並設定回歸原點的方式
```
10000 1 1 homing-a!
pp[] hm 33 1 1 4queue
pp[] inpos 0 1 1 4queue
\ 設定回歸原點加速度為10000
\ 表示回歸原點的方式為33
\ 此兩行指令的0皆可為任意數
\ 下hm模式的命令後需要再下inpos等待馬達到達定位的命令
\ 可以一次下多顆馬達的hm指令在下多顆馬達的inpos指令
```
* pp模式
```
\ 假設馬達的目標位置為10000，並假設速度為100000
pp[] pp-v 100000 1 1 4queue
pp[] pp 10000 1 1 4queue
pp[] inpos 0 1 1 4queue
\ inpos指令的0可為任意數
\ 下pp模式的命令後若下了inpos等待馬達到達定位的命令，則在到達定位之前不再接收其他指令
\ 可以一次下多顆馬達的pp-v指令再下多顆馬達的pp指令
\ 可以一次下多顆馬達的pp指令在下多顆馬達的inpos指令
```
* pv模式
```
\ 假設馬達的速度為10000
pp[] pv 10000 1 1 4queue
```
* acc指定加速度
```
\ 假設加速度為10000
pp[] acc 10000 1 1 4queue
```
* dec指定減速度
```
\ 假設減速度為10000
pp[] dec 10000 1 1 4queue
```
* 直接停止馬達
```
pp[] quickstop 0 1 1 4queue
\ 此處的0可為任意數
\ 下此指令後，當前執行的指令以及儲存在queue中的指令都會直接清除
```
* 減速停止馬達
```
pp[] slowstop 10000 1 1 4queue
\ 以減速度10000停止馬達
\ 下此指令後，當前執行的指令以及儲存在queue中的指令都會直接清除
```
* 設定從站alias
```
pp[] station-no-set 0 1 1 4queue
\ 此指令為把從站編號1的alias設為0
\ 此處的馬達位置可為任意數
```
這個指令有幾點要注意
1. alias 除了 0 以外，不可重複。
2. 此設定命令是修改 SII EEPROM 對應的暫存器。如果是由硬體旋鈕控制的，就不需要由此命令設定。
3. 不可以有重複的 alias。
4. 此命令會造成 Real Time Cycle Overrun。要在所有驅動器 Servo OFF 情況執行。

operation還有兩項指令，set-sdo以及get-sdo格式較為特別，故特別寫在這裡。

* SDO讀取
```
\ 想要讀取從站編號1的index $605D subindex 0 
\ 格式為 queue_name subindex index slave_num 4queue
pp[] get-sdo 0 $605D 1 4queue
\ 可讀取出index、subindex以及資料
```
* SDO寫入
```
\ 想要對從站編號1的index $605D subindex 0 寫入資料 100
\ 格式為 queue_name data index+subindex slave_num 4queue
\ index+subindex比較特殊，礙於格式的關係，在寫入資料的時候，需要把index以及subindex寫在一起，以這裡的例子即為$605D0，也就是index $605D + subindex 0
pp[] set-sdo 100 $605D0 1 4queue
\ 此指令可對指定位置寫入資料
```


# queue的構成

```
\ queue的基本元素
\ 最前面兩項為front、back，分別儲存從cell0開始算的最前端及最後端的索引值
\ 從back之後開始依序放入元素
\ +-------+------+--------+--------+--------+-----+
\ | front | back | cell 0 | cell 1 | cell 2 | ... |
\ +-------+------+--------+--------+--------+-----+

: create-queue ( capacity -- )
    create 0 , 0 , cells allot ;
100 constant #position        \ 陣列大小為 100
-1 constant serveroff         \ 將關閉指令命名為serveroff並賦值-1
0 constant serveron           \ 將開啟指令命名為serveron並賦值0
-2 constant inpos             \ 將等待抵達指令命名為inpos並賦值-2
-3 constant acc               \ 將指定加速度命令命名為acc並賦值-3
-4 constant dec               \ 將指定減速度命令命名為dec並賦值-4
-5 constant stop			  \ 將停止馬達命令命名為stop並賦值-5
-6 constant haltslow		  \ 將減速停止命令命名為haltslow -6
-7 constant haltquick		  \ 將直接停止命令命名為haltquick並賦值-7
-8 constant haltend			  \ 將停止暫停命令命名為haltend並賦值-8
-9 constant pp-v			  \ 將指定pp模式速度命令命名為pp-v並賦值-9
#position create-queue pp[]   \ 定義陣列名稱為 pp[]
variable dequeue_front 0 dequeue_front ! \ dequeue指令用的變數
variable dequeue_back 0 dequeue_back !   \ dequeue指令用的變數
variable slave 0 slave !      \ 儲存4queue裡面的slave_num
variable num 0 num !          \ 儲存4queue裡面的channel

\ 取得queue前端及尾端的索引
: front@ ( queue -- front )  @ ;
: back@ ( queue -- back )   cell+ @ ;

\ 判斷 queue 是否是空的，若為空回傳真值(-1)，反之回傳0
: empty? ( queue -- f ) dup front@  swap back@ = ;

\ 判斷 queue 是否是滿的，若為滿回傳真值(-1)，反之回傳0
: full? ( queue -- f ) dup back@ 1 + #position mod swap front@ = ;


\ 放一個元素進入queue中，此指令是為了方便2queue的建立。如果因為queue已滿無法放進去，傳一訊息給上位控制器。
: queue ( queue position -- )
    swap dup 
    full? not if
        dup -rot dup back@ 2 + cells + !
        dup back@ 1 + #position mod swap cell+ ! 
    else
        ." log|full!" cr
    then ;

\ 一次將兩個元素放進 queue 中。此指令為方便4queue的建立
: 2queue ( queue mode position -- )
    -rot 2dup queue
    drop swap queue ;

\ 一次將四個元素放進 queue 中。
: 4queue ( queue mode position slave n -- )
    pp[] -rot 2queue 2queue ;

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
```

4drive 的程式如下：


```
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
##### ###### 
```