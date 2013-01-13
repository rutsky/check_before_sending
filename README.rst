Check Before Sending
====================

Check spelling before sending message plugin for 
`Pidgin IM <https://pidgin.im/>`_.

When enabled this plugins behaves in the following way: if you try to send 
message containing spelling errors (words underlined with red wave), message 
will not be send and cursor will be positioned on first misspelled word.
If you try to send it again without modifications cursor will jump to next
misspelled word and so on, after skipping last misspelled word message is 
being sent.

Installation
-------

::

    git clone https://github.com/rutsky/check_before_sending.git check_before_sending
    mkdir build && cd build
    cmake -DCMAKE_INSTALL_PREFIX="${HOME}" ../check_before_sending
    make && make install

License
-------

Copyright (C) 2012  Vladimir Rutsky <altsysrq@gmail.com>

Distributed under GPLv2 (only option for Pidgin plugins) see COPYING.txt for 
details.
