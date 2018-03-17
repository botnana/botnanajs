extern crate botnanars;
use botnanars::Botnana;
use std::sync::{Arc, Mutex};

fn main() {
    let botnana = Arc::new(Mutex::new(Botnana::new().unwrap()));

    let btn = botnana.clone();
    botnana.lock().unwrap().once("ready", move |_| {
        let mut p = btn.lock().unwrap().program("p3");
        let mut s1 = p.ethercat.slave(1).unwrap();
        let mut s2 = p.ethercat.slave(2).unwrap();
        s1.hm();
        s2.hm();
        s1.go();
        s2.go();
        s1.pp();
        s2.pp();
        s1.move_to(30000);
        s2.move_to(40000);
        s1.go();
        s2.go();
        p.deploy();
    });

    let btn = botnana.clone();
    botnana.lock().unwrap().once("deployed", move |_| {
        let p = btn.lock().unwrap().program("p3");
        p.run();
    });

    botnana.lock().unwrap().start("ws://localhost:3012");
    loop {}
}