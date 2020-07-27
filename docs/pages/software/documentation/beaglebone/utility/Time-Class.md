🚧⚠️ This page is currently under construction ⚠️🚧

[[Wiki Home Page|Home]]

## Contents
1. [Quick Details](#quick_details)
2. [Description](#description)
3. [Member Functions](#member_functions)
4. [See Also](#see_also)

## Quick Details <a name="quick_details"></a>
|   Property   |      Value     |
| ------------ | -------------- |
| Source Files  | [TimeTools.h](https://github.com/mtmk-ee/artemis-cubesat-kit/blob/master/BeagleBone/include/utility/TimeTools.h) |

## Description <a name="description"></a>
The `Time` class provides a convenient way of telling the time, besides using your wrist watch.

Internally, this class holds the current UTC time as a Modified Julian Day. The same convention is used in [[COSMOS Web]] as well. 

## Member Functions <a name="member_functions"></a>

| Function | Description |
| -------- | ----------- |
| `Time()`   | Constructs a new `Time` object representing the current time |
| `static Time Now()` | Constructs a new `Time` object representing the current time |
| `void GetYMD(int &year, int &month, double &day)` | Converts the `Time` to a year, month, and fractional day |
| `string ToString()` | Converts the `Time` to an ISO 8601 formatted string representation |
| `double GetModifiedJulianDay()` | Returns the UTC time as a Modified Julian Day number |
| `double Seconds()` | Returns the number of Unix seconds represented by this time |
| `double Minutes()` | Returns the number of Unix minutes represented by this time |
| `double Hours()` | Returns the number of Unix hours represented by this time |
| `double Days()` | Returns the number of Unix days represented by this time |

## See Also <a name="see_also"></a>
* [[Timer Class]]
* [UTC](https://en.wikipedia.org/wiki/Coordinated_Universal_Time)
* [Julian Day](https://en.wikipedia.org/wiki/Julian_day)
* [Unix Time](https://en.wikipedia.org/wiki/Unix_time)
* [ISO 8601](https://en.wikipedia.org/wiki/ISO_8601)
