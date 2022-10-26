/* diskacc2.h - direct disk access library for OS/2 2.x protected mode.
 *
 * Author:  Kai Uwe Rommel <rommel@ars.muc.de>
 * Created: Fri Jul 08 1994
 */

/* $Id: diskacc2.h,v 1.3 1997/01/12 21:15:44 rommel Exp rommel $ */

/*
 * $Log: diskacc2.h,v $
 * Revision 1.3  1997/01/12 21:15:44  rommel
 * added CD-ROM routines
 *
 * Revision 1.2  1994/07/08 21:35:50  rommel
 * bug fix
 *
 * Revision 1.1  1994/07/08 21:34:12  rommel
 * Initial revision
 *
 */

int DskOpen(char *drv, int logical, int lock,
	    ULONG *sides, ULONG *tracks, ULONG *sectors);
int DskClose(int handle);
int DskRead(int handle, ULONG side, ULONG  track,
	    ULONG sector, ULONG nsects, void *buf);
int DskWrite(int handle, ULONG side, ULONG  track,
	     ULONG sector, ULONG nsects, void *buf);

int CDFind(int number);
int CDOpen(char *drv, int lock, char *upc, ULONG *sectors);
int CDClose(int handle);
int CDRead(int handle, ULONG sector, ULONG nsects, void *buf);

/* end of diskacc2.h */
