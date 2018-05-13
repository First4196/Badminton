# BadmintonEvolution
Badminton court and players detection using OpenCV

# How To Use
1.  Run these commands in terminal to build the project.
```bash
cmake .
make
```
2.  Put mp4 videos in **data/raws/** and name it **raw{name}.mp4**
3.  Run this command to execute the program and wait for program to finish
```bash
bash app {name}
```
    Or run this command to execute the program on several files
```bash
bash app {name1} {name2} {name3} ...
```
4.  Take result csvs from **data/csvs/csv{name}.csv**