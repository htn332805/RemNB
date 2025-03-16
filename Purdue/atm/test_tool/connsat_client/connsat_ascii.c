
#include "connsat_ascii.h"

#define NULL 0

int
get_ascii_control_code_info (
	unsigned char ascii_code,
	char **abrev_pp,
	char **desc_pp)
{
	int status;
	char *abrev;
	char *desc;

	status = 0;	/* ok */

	switch (ascii_code) {

		case 0x00:
			abrev = "NUL";
			desc = "null or time fill";

			break;

		case 0x01:
			abrev = "SOH";
			desc = "start of heading";

			break;

		case 0x02:
			abrev = "STX";
			desc = "start of text";

			break;

		case 0x03:
			abrev = "ETX";
			desc = "end of text";

			break;

		case 0x04:
			abrev = "EOT";
			desc = "end of transmission";

			break;

		case 0x05:
			abrev = "ENQ";
			desc = "enquiry";

			break;

		case 0x06:
			abrev = "ACK";
			desc = "acknowledge";

			break;

		case 0x07:
			abrev = "BEL";
			desc = "bell";

			break;

		case 0x08:
			abrev = "BS";
			desc = "backspace";

			break;

		case 0x09:
			abrev = "HT";
			desc = "horizontal tabulation";

			break;

		case 0x0a:
			abrev = "LF";
			desc = "line feed";

			break;

		case 0x0b:
			abrev = "VT";
			desc = "vertical tabulation";

			break;

		case 0x0c:
			abrev = "FF";
			desc = "form feed";

			break;

		case 0x0d:
			abrev = "CR";
			desc = "carriage return";

			break;

		case 0x0e:
			abrev = "SO";
			desc = "shift out";

			break;

		case 0x0f:
			abrev = "SI";
			desc = "shift in";

			break;

		case 0x10:
			abrev = "DLE";
			desc = "data line escape";

			break;

		case 0x11:
			abrev = "DC1";
			desc = "device control 1 (x-on)";

			break;

		case 0x12:
			abrev = "DC2";
			desc = "device control 2 (tape)";

			break;

		case 0x13:
			abrev = "DC3";
			desc = "device control 3 (x-off)";

			break;

		case 0x14:
			abrev = "DC4";
			desc = "device control 4 (tape)";

			break;

		case 0x15:
			abrev = "NAK";
			desc = "negative acknowledge";

			break;

		case 0x16:
			abrev = "SYN";
			desc = "synchronous idle";

			break;

		case 0x17:
			abrev = "ETB";
			desc = "end of transmission blocks";

			break;

		case 0x18:
			abrev = "CAN";
			desc = "cancel";
;
			break;

		case 0x19:
			abrev = "EM";
			desc = "end of medium";

			break;

		case 0x1a:
			abrev = "SUB";
			desc = "substitute";

			break;

		case 0x1b:
			abrev = "ESC";
			desc = "escape";

			break;

		case 0x1c:
			abrev = "FS";
			desc = "file separator";

			break;

		case 0x1d:
			abrev = "GS";
			desc = "group separator";

			break;

		case 0x1e:
			abrev = "RS";
			desc = "record separator";

			break;

		case 0x1f:
			abrev = "US";
			desc = "unit separator";

			break;

		case 0x7f:
			abrev = "DEL";
			desc = "delete";

			break;

		default:
			status = -1;

			break;

	}

	if (status == 0) {

		if (abrev_pp != NULL) {
			*abrev_pp = abrev;
		}

		if (desc_pp != NULL) {
			*desc_pp = desc;
		}
	}

	return status;
}

