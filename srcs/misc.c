#include <stddef.h>

int	ft_strcmp(const char *s1, const char *s2)
{
	int				i;
	unsigned char	*str1;
	unsigned char	*str2;

	str1 = (unsigned char *)s1;
	str2 = (unsigned char *)s2;
	i = 0;
	while (str1[i] == str2[i] && str1[i] && str2[i])
		i++;
	return (str1[i] - str2[i]);
}

void	*ft_memcpy(void *dst, const void *src, size_t n)
{
	char	*str;
	char	*dest;
	int		i;

	i = 0;
	str = (char *)src;
	dest = (char *)dst;
	while (n)
	{
		dest[i] = str[i];
		n--;
		i++;
	}
	return (dst);
}
