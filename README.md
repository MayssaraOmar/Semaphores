The system recieves random independent messages coming in independent threads, each increment the same counter.<br /> 
A Monitor thread  get the current value of the counter, resets it to zero and places this value in a buffer of a fixed size at random time intervals.<br />
A Collector thread reads the values from the buffer at random time intervals.<br /><br />
Any  thread  will  have  to  wait  if  the  counter  is  being  locked  by  any  other  thread.<br /> 
The Monitor and Collector threads will not be able to access the buffer at the same time.<br />
The Monitor can't add another entry if the buffer is full.<br />
The Collector can't read from an empty buffer.<br /><br /> 
The random incoming messages to the system are simulated by setting their corresponding threads to
sleep for random times, and their activation (sleep time ends) corresponds to an email arrival. 
