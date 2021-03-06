/** CSci-4611 Assignment 3:  Earthquake
 */

#ifndef EARTHQUAKE_H_
#define EARTHQUAKE_H_

#include <string>

#include "date.h"

/** Small data structure for storing earthquake data.
 */
class Earthquake {
public:
    // Create an earthquake from a datafile's line of text
    Earthquake(std::string s);
    Earthquake();

    Date date();

    double longitude();

    double latitude();

    double magnitude();
    
    /*double ratio();
    
    void setRatio(const double &r);*/
    
    //double size_ratio;

private:

    double ParseFloat(std::string s);
    int ParseInt(std::string s);
    double size_ratio;

    std::string line;
};

#endif
