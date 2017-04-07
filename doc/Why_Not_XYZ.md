# Why not XYZ but anothr project generator ?
I think this is the most common question people asked me.

The answer is simple, it's because no other generator fit what I want.
Of cause every generator has their own pros and crons, but one size doesn't fit all.

After years of looking for my favourite generator, I still couldn't find single one I happy with, 
so at the end I decided to make one, it may not be perfect, but at least it fit what I really need.

And I'm not trying make one to replace others, so if you happy with what you are using, 
just keep doing,but if not, you can try to take a look of this one.

## Comparsion to other generator
* [Why not CMake](Why_Not_CMake.md)

## Conclusion
In fact if you see how people using Visual C++ or Xcode, they don't have any problem about 
how to manage files or compile option in project, and it's all the information compiler need.

For me I'm happy with those IDE, but the only problem I have is cross-platform, 
when I try to maintain a project which have to compile on Windows, Mac, Linux, FreeBSD, iOS, Android ...etc, and they all using different IDE, it became a nightmare to me

Therefore I make this project generator, just read a json file for all information is 
needed then output target IDE project for my actual work.