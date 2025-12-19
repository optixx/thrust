
/* Written by Peter Ekberg, peda@lysator.liu.se */

#ifndef CONF_H
#define CONF_H

extern int scancode[];
extern char *keynames[];

void initkeys(void);
void conf(void);
int getscancode(int old, int x, int y);

#endif /* CONF_H */
