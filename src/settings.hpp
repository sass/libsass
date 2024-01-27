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
#define SassDefaultEpsilon 10E-10
#define SassDefaultNrSprintf "%.10f"

/////////////////////////////////////////////////////////////////////////
// Hard-coded maximum nesting until we bail out.
// Note that this limit is not an exact science
// it depends on various factors, which some are
// not under our control (compile time or even OS
// dependent settings on the available stack size)
// It should fix most common segfault cases though.
/////////////////////////////////////////////////////////////////////////

#ifndef SassMaxNesting
#define SassMaxNesting 512
#endif

/////////////////////////////////////////////////////////////////////////
// Should we preserve color information when possible
// E.g. if we create a HWB color with zero hue (which is always `gray`),
// dart-sass returns `50.196%` for its whiteness, no matter with which
// `whiteness` argument the HWB color was initialized. Even worse that
// `gray` would actually have a `whiteness` of exactly `50%`. But due
// to dart-sass internally clamping the rgb components to integer, the
// wrong `whiteness` of `50.1960784314%` is produced. LibSass tries to
// preserve the information for colors whenever possible, when doing
// transformations between formats it will not clamp the components.
/////////////////////////////////////////////////////////////////////////

#ifndef SassPreserveColorInfo
#define SassPreserveColorInfo 1
#endif

/////////////////////////////////////////////////////////////////////////
// Sort map keys when outputting via inspect et al
/////////////////////////////////////////////////////////////////////////

#ifndef SassSortMapKeysOnOutput
#define SassSortMapKeysOnOutput 0
#endif

/////////////////////////////////////////////////////////////////////////
// Error when extending compound selectors
/////////////////////////////////////////////////////////////////////////

// Disable for older LibSass 3.6.3 behavior
#ifndef SassRestrictCompoundExtending
#define SassRestrictCompoundExtending 1
#endif

/////////////////////////////////////////////////////////////////////////
// Logger default settings
/////////////////////////////////////////////////////////////////////////

// Default output character columns
#ifndef SassDefaultColumns
#define SassDefaultColumns 120
#endif

/////////////////////////////////////////////////////////////////////////
// Optional static hash seed
/////////////////////////////////////////////////////////////////////////

// Define static hash seed (random otherwise)
// 0x9e3779b9 is the Fibonacci/Golden Ratio
// #define SassStaticHashSeed 0x9e3779b9

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

// Number of references until we can safely self assign.
// Set to a zero to practically disable this feature.
#ifndef AssignableRefCount
#define AssignableRefCount 3
#endif

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

#endif
