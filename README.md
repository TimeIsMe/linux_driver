# linux_driver
This is my workplace for practice the Linux driver.

***  

## sub directory

---new_template It's a new template linux device driver, no real options to registers.  

---dtsled	It's a led device driver based on device-tree for imx6ull SOC.  

---pinctrl_gpioSubSys It's a led device driver based on pinctrl and gpio sub system.  

---pinctrl_beep It's modified from pinctrl_gpioSubSys.  

---key		It's modified from pinctrl_gpioSubSys with atomic operation.

---timer	It's modified from pinctrl_gpioSubSys with linux kernel timer.  

---irqkey	There is atomic operation, kernel timer and interrupt for key.  

---blockio	Inherit from irqkey, and change the app poll to block read.  

---noblockio	Inherit from blockio, and change block read to nonblock poll.  

---asyncnoti	Inherit from noblockio, and add the asynchronous notifications.  

---platform_ndt	Platform driver without device tree.  

---misc_driver	misc char driver.  

---interprocess_communication	It's a directory about interprocess communication.  
&nbsp;&nbsp;&nbsp;|---1_pipe	It's implement interprocess communication using pipe.  
&nbsp;&nbsp;&nbsp;|---2_named_pipe	It's implement interprocess communication using named pipe.  
&nbsp;&nbsp;&nbsp;|---3_message_queue	It's implement interprocess communication using message queue.  
&nbsp;&nbsp;&nbsp;|---4_shared_memory It's implement interprocess communication using shared memory.  
&nbsp;&nbsp;&nbsp;|---5_semaphore     It's implement interprocess communication using semaphore.  
