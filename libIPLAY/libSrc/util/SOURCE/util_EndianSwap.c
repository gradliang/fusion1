// util_EndianSwap.c

void EndianChange16(unsigned short *value)
{
	unsigned short victim;
	victim = *value;
	*value = (victim >> 8) + (victim << 8);
}

void EndianChange32(unsigned int *value)
{
	unsigned int victim;
	victim = *value;
	victim = (victim >> 24) + ((victim & 0x00ff0000) >> 8) +
            ((victim & 0x0000ff00) << 8) + (victim << 24);
    *value = victim;
}