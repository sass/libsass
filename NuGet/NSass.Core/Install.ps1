param($installPath, $toolsPath, $package, $project)

$name = "NSass.Wrapper"

$x86 = $project.ProjectItems.Item($name).ProjectItems.Item($name + ".x86.dll")
$x86.Properties.Item("CopyToOutputDirectory").Value = 2;

$x64 = $project.ProjectItems.Item($name).ProjectItems.Item($name + ".x64.dll")
$x64.Properties.Item("CopyToOutputDirectory").Value = 2;