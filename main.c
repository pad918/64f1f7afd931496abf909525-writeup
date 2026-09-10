#include <stdio.h>
#include <inttypes.h>
#include <time.h>
#include <string.h>
#include <openssl/evp.h>

/*
    Represents the state of the program
*/
typedef struct rng_state{
    uint32_t next;
    char password[0x20];
    int password_idx;

} rng_state;

/*
    The state as it is in the start of the
    program
*/
static const rng_state init_state = {
    .next = 1,
    .password = {0},
    .password_idx = 0
};

// The program uses a chacha20 IV of just zeros
static const uint8_t nonce[12] = {0};

/* Copied directly from Ghidra
    you can select copy ==> as C string.
*/
static const char cipher_text[] = { 
    0xf4, 0xb8, 0xc0, 0x90, 0xcd, 0xf6, 0xcb, 
    0xf7, 0xc5, 0x42, 0x30, 0xf3, 0x75, 0x04, 
    0x2c, 0x4b, 0xfd, 0x9d, 0x96, 0x02, 0x57, 
    0x79, 0x9a, 0x7a, 0xee, 0x75, 0xd9, 0xf0, 
    0x4c }; 

void chacha20_decrypt(const uint8_t key[32], const uint8_t nonce[12],
                      const uint8_t *ciphertext, size_t n,
                      uint8_t *plaintext) {
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    int outlen;

    EVP_DecryptInit_ex(ctx, EVP_chacha20(), NULL, key, nonce);
    EVP_DecryptUpdate(ctx, plaintext, &outlen, ciphertext, (int)n);
    EVP_CIPHER_CTX_free(ctx);
}

int current_fruit_x=0, current_fruit_y=0;

void update_password(rng_state* state){
    state->password[state->password_idx] = 
        (char)current_fruit_x + state->password[state->password_idx];
    state->password[state->password_idx] = 
        state->password[state->password_idx] * (char)current_fruit_y;
    state->password_idx = (state->password_idx + 1) % 0x20;
}

void __srand(rng_state* state, uint64_t seed){
    state->next = seed;
}

uint32_t __rand(rng_state* state){
    state->next = state->next * 0x41c64e6d + 0x3039;
    return (state->next >> 0x10) & 0x7fff;
}

void add_fruit(rng_state* state){
    current_fruit_y = __rand(state);
    current_fruit_y = current_fruit_y % 0x12 + 1;
    current_fruit_x = __rand(state);
    current_fruit_x = current_fruit_x % 0x26 + 1;
}

// Returns true if flag found
int test_decrypt(int rounds, uint64_t seed, char* decrypted){
    rng_state state = init_state;
    __srand(&state, seed);

    add_fruit(&state);

    // Apply the rounds, each round modifies
    // the password slightly.
    for(int i=0; i<rounds; i++){
        update_password(&state);
        add_fruit(&state);
    }

    chacha20_decrypt(state.password, nonce, cipher_text, 0x1d, decrypted);

    // The assembly already tells us the four first characters of the plain text
    if(strncmp(decrypted, "brb{", 4)!=0)
        return 0;
    return 1;
}

int main(){
    // start_time = time(NULL)
    // seed = start_time + (start_time / 20000) * -20000
    // ==> seed is in range [0, 20k]
    const int RANGE_UPPER_LIMIT = 20000;

    // +1 for null terminator
    char decrypted[0x1d+1] = {0}; 

    for(int i=0; i<RANGE_UPPER_LIMIT; i++){
        // The game ends when the snake's length is 0x226 = 550, or after
        // 550-initial_length = 550-3 = 547 rounds are applied.
        if(test_decrypt(547, i, decrypted)){
            printf("FOUND THE FLIPPIN FLAG: %s\n", decrypted);
            exit(0);
        } 
    }

    
}

// FLAG: brb{a_snake_and_20_chachas}