// WARNING: Please don't edit this file. It was generated by C++/WinRT v2.0.230706.1

void* winrt_make_ZipShellExt_ExtractToCommand()
{
    return winrt::detach_abi(winrt::make<winrt::ZipShellExt::factory_implementation::ExtractToCommand>());
}
WINRT_EXPORT namespace winrt::ZipShellExt
{
    ExtractToCommand::ExtractToCommand() :
        ExtractToCommand(make<ZipShellExt::implementation::ExtractToCommand>())
    {
    }
}