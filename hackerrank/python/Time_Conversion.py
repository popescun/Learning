#!/bin/python3

import math
import os
import random
import re
import sys
from logging import exception


#
# Complete the 'timeConversion' function below.
#
# The function is expected to return a STRING.
# The function accepts STRING s as parameter.
#

def timeConversion(s: str):
    # Write your code here

    day_part = s[len(s) - 2:len(s):]
    hh, mm, ss  = s[0:len(s) - 2:].split(':')

    if day_part == 'AM':
        # 12AM -> 11AM, 00 -> 11
        hh = int(hh)
        hh = 00 if hh == 12 else hh
        if hh > 11:
            raise exception('Invalid hh')
        return f'{hh:02}:{mm}:{ss}'
    else:
        # 12PM -> 11PM =  12 -> 23
        if day_part == 'PM':
            hh = int(hh)
            if hh != 12 and hh > 11:
                raise exception('Invalid hh')
            hh = hh if hh == 12 else hh + 12
            return f'{hh:02}:{mm}:{ss}'

    return ''

if __name__ == '__main__':
    # fptr = open(os.environ['OUTPUT_PATH'], 'w')

    # s = input()

    s = '12:45:54PM'

    result = timeConversion(s)

    print(result)

    # fptr.write(result + '\n')

    # fptr.close()
