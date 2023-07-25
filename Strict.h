#pragma once

/*
  A way to define a typedef similar class that is stricter than a typedef.
 */

template class <typename T> strict_typedef: public T
{
};
