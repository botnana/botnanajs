extern crate botnanars;
use botnanars::Botnana;
use std::sync::{Arc, Mutex};

fn main() {
    let botnana = Arc::new(Mutex::new(Botnana::new().unwrap()));

    let btn = botnana.clone();
    botnana.lock().unwrap().once("ready", move |_| {
        let script = "7 reset-fault 1 7 pds-goal";
        btn.lock().unwrap().evaluate(script);
    });

    botnana.lock().unwrap().start("ws://192.168.50.197:3012");

    loop {}
}