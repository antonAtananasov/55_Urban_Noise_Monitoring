size_t F2Len(const __FlashStringHelper *ifsh)
{
    PGM_P p = reinterpret_cast<PGM_P>(ifsh);
    size_t n = 0;
    while (1)
    {
        unsigned char c = pgm_read_byte(p++);
        if (c == 0)
            break;
        n++;
    }
    return n;
}

String F2Scharr(const __FlashStringHelper *text, byte length)
{
    char buffer[length];
    strncpy_P(buffer, (const char *)text, length - 1); // _P is the version to read from program space
    return String(buffer);
}
