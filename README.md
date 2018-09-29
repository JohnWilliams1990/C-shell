Author: John Williams 
Student ID: 105201054
                       
                       
I used the resources from the book and by watching a few of Dr. Brian Frasiers videos of forking to get a general understanding of forking. Please see his youtube channel...


Inorder to build the file simply issue the make command. 

I tested this program against alot of different commands involving piping and on the CSE grid found a few issues that I have hopefully fixed. The memory was an issue that I believe I found the answer for. When exec executes it calls exit() which erases all of the memory of the child proces that was available, including the memory that was dynamic for the parent. 

running the program is ./myshell

ls -la | grep -v grinch 
ls -la | wc 
ls -la > foo 
sort < foo
ls -la | grep my

cd 
pwd

and a host of other commands work for this program..


Enjoy...
 



