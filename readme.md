# Solution and Writeup for crackme 64f1f7afd931496abf909525 "s4r's snake"

## Step 0: Unpacking
Analysing the raw snake game file in ghidra clearly showed that a packer had been used, making analysis impossible. By searching for simple strings in the file, I found the type of packer used to pack the executable:

```bash
$ strings -n 10 snake
```

Output:

```
...
$Info: This file is packed with the UPX executable packer http://upx.sf.net $
$Id: UPX 3.96 Copyright (C) 1996-2020 the UPX Team. All Rights Reserved. $
/proc/self/exe
GCC: (Ubuntu 11.3.0-1u
...
```

The program can be unpacked with the UPX program found at "https://github.com/upx/upx".

```
upx -d ./snake
```

After unpacking we can see symbols

```
$ nm snake | grep "main"
                 U __libc_start_main@GLIBC_2.34
0000000000004a54 T main
```


## Step 1: Initial Reversing Findings



## Step 2: Obtaining the key