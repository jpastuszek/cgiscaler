/***************************************************************************
 *   Copyright (C) 2007 by Jakub Pastuszek   *
 *   jpastuszek@gmail.com   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

/* enable debuging - not suitable for production env! */
//#define DEBUG
/* sync every line written to debug file... slow */
//#define DEBUG_SYNC


/** Command-line configuration */
#define COMMANDLINE_WIDTH_SWITCH "w"
#define COMMANDLINE_HEIGHT_SWITCH "h"
#define COMMANDLINE_STRICT_SWITCH "s"
#define COMMANDLINE_LOWQ_SWITCH "wap"
#define COMMANDLINE_NO_CACHE_SWITCH "nc"
#define COMMANDLINE_NO_SERVE_SWITCH "ns"
#define COMMANDLINE_NO_HEADERS_SWITCH "nh"

#define COMMANDLINE_TRUE_VAL "true"
#define COMMANDLINE_FALSE_VAL "false"

/** Disk/cache file serving buffer */
#define WRITE_BUFFER_LEN 8192
