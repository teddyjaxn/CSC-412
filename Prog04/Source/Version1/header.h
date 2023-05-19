/// @file header.h

/// Standardized name template for files which store the file list for each process to distribute.
#define FIRST_OUTPUT_PREFIX "./temp/unsorted_data_"

/// Standardized name template for files containing each process' distributed lfile content
#define SECOND_OUTPUT_PREFIX "./temp/assigned_data_"

/// Standardized name template for files once they have been sorted by line number
#define THIRD_OUTPUT_PREFIX "./temp/finished_data_"

/** 
 * Calculates the length of an integer when converted to a string
 * @param integer
 * @return length in bytes
 **/
int intToStrLen(int n)
{
  int length = 1;
  for( length = 1; n >= 10; length++){
    n = n/10;
    if(length > 6){
      return 7;
    }
  } 
  return length;
}



