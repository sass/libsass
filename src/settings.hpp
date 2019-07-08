/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#ifndef SASS_SETTINGS_H
#define SASS_SETTINGS_H

// Global compile time settings should go here
// These settings are meant to be customized

/////////////////////////////////////////////////////////////////////////
// Sass default settings
/////////////////////////////////////////////////////////////////////////

// Default precision to format floats
#define SassDefaultPrecision 10

/////////////////////////////////////////////////////////////////////////
// Logger default settings
/////////////////////////////////////////////////////////////////////////

// Default output character columns
#define SassDefaultColumns 80

/////////////////////////////////////////////////////////////////////////
// Optional static hash seed
/////////////////////////////////////////////////////////////////////////

// Define static hash seed (random otherwise)
// #define STAStaticHashSeed 0x9e3779b9

/////////////////////////////////////////////////////////////////////////
// Optimization configurations
/////////////////////////////////////////////////////////////////////////

// When enabled we use our custom memory pool allocator
// With intense workloads this can double the performance
// Max memory usage mostly only grows by a slight amount
// Brings up to 50% improvement with minor memory overhead.
#define SASS_CUSTOM_ALLOCATOR

// Elide unnecessary value copies in eval, as we don't need to
// make copies already in the eval stage (I think). All further
// operations on those values will create a copy anyway!
// This applies to Number, String, Color and Boolean.
// Brings up to 5% improvement
#define SASS_ELIDE_COPIES

/////////////////////////////////////////////////////////////////////////
// Self assign optimization is experimental and may break your code
/////////////////////////////////////////////////////////////////////////
// Optimize self assign use-case where certain function would create
// a new copy and then assign to ourself, we can e.g. optimize map-merge
// or list-append in the following case: `$map = map-merge($map, $other)`.
// In order to do this we scan if on any assignment the right hand side
// is a function with the same variable as the first arguments. This flag
// is passed to the function when executed, so it knows to alter in-place.
/////////////////////////////////////////////////////////////////////////
#define SASS_OPTIMIZE_SELF_ASSIGN

// Number of references to safely self assign
#define AssignableRefCount 3

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

#endif
