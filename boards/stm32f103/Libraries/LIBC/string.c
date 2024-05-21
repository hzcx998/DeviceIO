int strcmp(const char * s1, const char * s2)
{
	int res;

	while (1)
	{
		if ((res = *s1 - *s2++) != 0 || !*s1++)
			break;
	}
	return res;
}
