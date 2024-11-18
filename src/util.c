unsigned int state = 1804289383; // random starting number

unsigned int generate_random_number() {
    unsigned int num = state;

    // XOR shift 32 algorithm
    num ^= num << 13;
    num ^= num >> 17;
    num ^= num << 5;

    // update state
    state = num;
    return num;
}