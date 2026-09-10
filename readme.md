# Solution and Writeup for crackme "s4r's snake"
crackme link https://crackmes.one/crackme/64f1f7afd931496abf909525

## Step 0: Unpacking
Analyzing the raw snake game file in Ghidra clearly showed that a packer had been used, making analysis impossible. By searching for simple strings in the file, I found the type of packer used to pack the executable:

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
After loading the unpacked binary into Ghidra I first tried to find the code where the flag is printed, which I found in the end of the function update_screen

```c
if (game_result == -0x5e) {
    chacha20_init_context(chacha_ctx,password,chacha_nonce,0);
    chacha20_xor(chacha_ctx,&encrypted_flag,0x1d);
    if ((((encrypted_flag != 'b') || (DAT_00107031 != 'r')) || (DAT_00107032 != 'b')) ||
        (DAT_00107033 != '{')) {
        snprintf(&encrypted_flag,0x1d,"%s","No flag for you today...");
    }
...
```

Here I identified some key global variables:

* encrypted_flag: The flag encrypted with chacha20
* password: The chacha20 32B key used for encryption / decryption.
* chacha_nonce: The IV used in chacha20, has initial value 0 and is never written to

So by obtaining the password, it should be possible to obtain the flag. Thus I continued by investigating what how the value of the password changed. 

The password is only written to from the eat_fruit function as follows

```c
password[password_idx] = (char)current_fruit_x + password[password_idx];
password[password_idx] = password[password_idx] * (char)current_fruit_y;
password_idx = (password_idx + 1) % 0x20;
```
Here we find three new variables that affect the value of the password:

* password_idx: used to update different parts of the password each time an apple is eaten.
* current_fruit_x: X position of current fruit, randomly updated after each eaten fruit
* current_fruit_y: Y position of current fruit, randomly updated after each eaten fruit

The position of the fruit is randomly set in the add_fruit function that is called once in the beginning of the program and after each new eaten fruit. The PRNG that is super basic and is defined as such:

```c
// Set the seed
void srand(uint __seed)
{
  next = (ulong)__seed;
  return;
}
```

```c
int rand(void)
{
  next = next * 0x41c64e6d + 0x3039;
  return (uint)((ulong)next >> 0x10) & 0x7fff;
}
```

If we only know the initial seed, and the number of total apples eaten, we should now be able to obtain the password. There is however an issue here, the seed is not a constant, but is based on the current unix time modulo 20k. This means that there are 20k possible initial seeds, but only one of them is correct.


## Step 2: Obtaining the key
To obtain the key I wrote a C program that implements the same logic as the snake program and try to decrypt the flag for each of the possible 20k initial seeds. We can determine if the flag is valid by comparing the decrypted value with the 4 first characters of the flag that are reviled in the binary. 


## How to run
```bash
./run.sh
```

Only works on Linux