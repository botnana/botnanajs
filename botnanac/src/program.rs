use botnanars::{botnana::Botnana, program::Program};
use std::{ffi::CStr, os::raw::c_char, str};

#[no_mangle]
pub extern "C" fn program_new(name: *const c_char) -> Box<Program> {
    let name = unsafe {
        assert!(!name.is_null());
        str::from_utf8(CStr::from_ptr(name).to_bytes()).unwrap()
    };
    Box::new(Program::new(name))
}

#[no_mangle]
pub extern "C" fn program_new_with_file(name: *const c_char, path: *const c_char) -> Box<Program> {
    let name = unsafe {
        assert!(!name.is_null());
        str::from_utf8(CStr::from_ptr(name).to_bytes()).unwrap()
    };
    let path = unsafe {
        assert!(!path.is_null());
        str::from_utf8(CStr::from_ptr(path).to_bytes()).unwrap()
    };
    Box::new(Program::new_with_file(name, path))
}

/// deploy porgram
#[no_mangle]
pub extern "C" fn program_deploy(botnana: Box<Botnana>, program: Box<Program>) {
    let program = Box::into_raw(program);
    let s = Box::into_raw(botnana);
    unsafe { (*s).program_deploy(&mut (*program)) };
}

/// push program line
#[no_mangle]
pub extern "C" fn program_line(program: Box<Program>, cmd: *const c_char) {
    let program = Box::into_raw(program);
    let cmd = unsafe {
        assert!(!cmd.is_null());
        String::from_utf8_lossy(CStr::from_ptr(cmd).to_bytes())
    };

    unsafe {
        (*program).push_line(&cmd.into_owned());
    }
}

/// clear program
#[no_mangle]
pub extern "C" fn program_clear(program: Box<Program>) {
    let program = Box::into_raw(program);
    unsafe {
        (*program).clear();
    }
}

/// run porgram
#[no_mangle]
pub extern "C" fn program_run(botnana: Box<Botnana>, program: Box<Program>) {
    let program = Box::into_raw(program);
    let s = Box::into_raw(botnana);
    unsafe { (*s).program_run(&(*program)) };
}
