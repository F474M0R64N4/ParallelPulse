use std::sync::{Arc, Mutex, Condvar};
use std::thread;
use std::time::Duration;
use std::sync::mpsc;

pub struct ThreadGroup {
    threads: Vec<thread::JoinHandle<()>>,
    sender: mpsc::Sender<()>,
    closed: Arc<(Mutex<bool>, Condvar)>,
}

impl ThreadGroup {
    pub fn new() -> Self {
        let (sender, receiver) = mpsc::channel();
        let closed = Arc::new((Mutex::new(false), Condvar::new()));

        std::thread::spawn({
            let closed = Arc::clone(&closed);
            move || {
                receiver.recv().unwrap();
                let (lock, cvar) = &*closed;
                let mut closed = lock.lock().unwrap();
                *closed = true;
                cvar.notify_all();
            }
        });

        ThreadGroup {
            threads: Vec::new(),
            sender,
            closed,
        }
    }

    pub fn add_thread<F>(&mut self, thread_function: F) -> bool
    where
        F: FnOnce() + Send + 'static,
    {
        let (lock, _) = &*self.closed;
        let closed = lock.lock().unwrap();

        if *closed {
            return false;
        }

        let handle = thread::spawn(thread_function);
        self.threads.push(handle);
        true
    }

    pub fn wait_for_all_exit(&self, timeout: Option<Duration>) -> bool {
        let (lock, cvar) = &*self.closed;
        let mut closed = lock.lock().unwrap();

        if *closed {
            return true;
        }

        match timeout {
            Some(duration) => {
                let result = cvar.wait_timeout(closed, duration).unwrap();
                *result.0
            }
            None => {
                closed = cvar.wait(closed).unwrap();
                *closed
            }
        }
    }

    pub fn close_group(&mut self) {
        let _ = self.sender.send(());

        for thread in self.threads.drain(..) {
            let _ = thread.join();
        }
    }
}

impl Drop for ThreadGroup {
    fn drop(&mut self) {
        self.close_group();
    }
}
