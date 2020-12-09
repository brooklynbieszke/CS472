•How the code is implemented? What are main function calls? 

Structs:
output - compressed output
buffer - page data
fd - munmap file data
buffer get - gets the old jobs to put them into the next job

Functions:
put - gets the current job and puts in into the queue
producer- produces jobs to input into the queue
makeResult- makes the result and prints it to pass the tests
clean - frees and closes anything open to prevent memory leaks
main

•How did you test the code to make sure the correctness of the code? 

We used valgrind and also generic debugging for errors and warnings that appeared.

•3If you work with a partner, who is your group member? How much code was contributed by you, and how much was contributed by your group member? Which functions were implemented you or your group member

Group: Chase Lose and Brooke Bieszke

Brooke:

Implemented the producer and consumer into the project

Chase:

Formatted the previous code to allow the producer and consumer to work in the project as well as assisted with the producer and consumer in the project