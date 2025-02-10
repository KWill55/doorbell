# Doorbell

## Experience Summary

### How many hours did it take to complete assignment?
5

### What did you enjoy about this lab?
I liked that it allowed us to put all the pieces together to create the whole doorbell!

### What were the major challenges you had with this lab? Try to be as detailed as possible.
I was having problems because I put an extra / in my absolute address which made me check a bunch of other stuff until I realized where I messed up haha. 

## Lab Specific Tasks

### systemd Daemons
1. What is a daemon?

A daemon is a program that runs in the background and is not directly started by the user.

2. What is `systemd`?

It manages the processes of Linux. 

3. What program controls `systemd` daemons?

systemd

4. What kind of file is made to create a daemon in `systemd`?

Service Files

5. Explain the difference between systemctl start and systemctl enable. When would you use one over the other?

Systemctl start is useful for starting the service immediately. System enable allows the application to run automatically once rebooted.

6. Can you think of another real-world application where you might want a program to start automatically on boot? Describe the scenario.

I think lots of things are like this! Once you plug in a refrigorator, you want it to start running. The same with most appliances that people use. Additionally, I'm sure that things like traffic lights use similar things, because they want to turn on immediately once installed. 

7. Do a little research on your own and list 3 different daemons that run by default in Linux. Why are those programs better as daemons than normal programs?

sshd (openSSH Daemon): handles incoming ssh connections
cron (cron Daemon): executres scheduled tasks at predefined times and intervals. 
ryslogd (RSylog Daemon): manages syhstem logs in Linux
NetworkManager: provides network connectivity management for Linux based systems. 

They work better as Daemons because it means that they're always avaialbel to perform their specific task. It makes the whole system much more usable and convenient. 
