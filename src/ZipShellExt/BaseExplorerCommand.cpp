#include "pch.h"
#include "BaseExplorerCommand.h"

#include <codecvt>

#include "BaseExplorerCommand.g.cpp"
#include <fstream>

namespace winrt::ZipShellExt::implementation
{
	/// <summary>
	/// IExplorerCommand �ӿڵ���ϸ���ܣ�https://learn.microsoft.com/zh-cn/windows/win32/api/shobjidl_core/nn-shobjidl_core-iexplorercommand
	/// ��Ӳ˵�����
	/// 1.��Ҫ��BaseExplorerCommand.idl�ж���˵������SevenZipCommand
	/// 2.��BaseExplorerCommand.h�������Ӧ�Ľӿ�ʵ�֣�����SevenZipCommand��ʹ�õ��ĸ�����������Ӧ��������ʵ���ĸ�������������������ʵ����GetTitle()����������������ʵ��GetTitle()����
	/// 3.��BaseExplorerCommand.h�����ʵ���������ɵ��ļ�������SevenZipCommand�����Ӧ���ļ���SevenZipCommand.g.h����Ҫ����ͷ�ļ� "SevenZipCommand.g.h"
	/// 4.��BaseExplorerCommand.cpp��ʵ�ֶ�Ӧ�ķ���
	/// 5.�����Ҫ�ڸò˵�����Ӷ����˵�������Ҫʵ�������SubCommands()��������Ϊ�������Ӧ�����ݣ��ο�SevenZipCommand�е�ʵ��
	/// 6.�ò˵����û��������Ҫ�����û���������ݣ��û�����󷵻ص�������Invoke()�����У�����IShellItemArray�а�������˵�ѡ�е�����
	/// </summary>
	
	/// <summary>
	/// SubMenu ����
	/// </summary>
	IFACEMETHODIMP SubMenu::Next(ULONG celt, __out_ecount_part(celt, *pceltFetched) IExplorerCommand** apUICommand, __out_opt ULONG* pceltFetched)
	{
		const uint32_t oldIndex = mIndex;
		const uint32_t endIndex = mIndex + celt;
		const uint32_t commandCount = mCommands.Size();
		for (; mIndex < endIndex && mIndex < commandCount; mIndex++)
		{
			mCommands.GetAt(mIndex).try_as<IExplorerCommand>().copy_to(apUICommand);
		}

		const uint32_t fetched = mIndex - oldIndex;
		ULONG outParam = static_cast<ULONG>(fetched);
		if (pceltFetched != nullptr)
		{
			*pceltFetched = outParam;
		}
		return (fetched == celt) ? S_OK : S_FALSE;
	}

	IFACEMETHODIMP SubMenu::Skip(ULONG /*celt*/)
	{
		return E_NOTIMPL;
	}

	IFACEMETHODIMP SubMenu::Reset()
	{
		mIndex = 0;
		return S_OK;
	}

	IFACEMETHODIMP SubMenu::Clone(__deref_out IEnumExplorerCommand** ppenum)
	{
		*ppenum = nullptr; return E_NOTIMPL;
	}

	/// <summary>
	/// BaseExplorerCommand ����
	/// </summary>
	BaseExplorerCommand::BaseExplorerCommand() {};

	IFACEMETHODIMP BaseExplorerCommand::GetTitle(_In_opt_ IShellItemArray* items, _Outptr_result_nullonfailure_ PWSTR* name)
	{
		winrt::hstring title = L"";
		return SHStrDup(title.c_str(), name);
	}

	IFACEMETHODIMP BaseExplorerCommand::GetIcon(_In_opt_ IShellItemArray*, _Outptr_result_nullonfailure_ PWSTR* icon)
	{
		winrt::hstring iconPath = L"";
		return SHStrDup(iconPath.c_str(), icon);
	}

	IFACEMETHODIMP BaseExplorerCommand::GetToolTip(_In_opt_ IShellItemArray*, _Outptr_result_nullonfailure_ PWSTR* infoTip)
	{
		*infoTip = nullptr;
		return E_NOTIMPL;
	}

	IFACEMETHODIMP BaseExplorerCommand::GetCanonicalName(_Out_ GUID* guidCommandName)
	{
		*guidCommandName = GUID_NULL;
		return S_OK;
	}

	IFACEMETHODIMP BaseExplorerCommand::GetState(_In_opt_ IShellItemArray* selection, _In_ BOOL okToBeSlow, _Out_ EXPCMDSTATE* cmdState)
	{
		ExplorerCommandState state = ExplorerCommandState::Disabled;
		*cmdState = static_cast<EXPCMDSTATE>(state);
		return S_OK;
	}

	IFACEMETHODIMP BaseExplorerCommand::Invoke(_In_opt_ IShellItemArray* selection, _In_opt_ IBindCtx*)
	{
		return S_OK;
	}

	IFACEMETHODIMP BaseExplorerCommand::GetFlags(_Out_ EXPCMDFLAGS* flags)
	{
		auto subCommands = SubCommands();
		bool hasSubCommands = subCommands != nullptr && subCommands.Size() > 0;
		*flags = !hasSubCommands ? ECF_DEFAULT : ECF_HASSUBCOMMANDS;
		return S_OK;
	}

	IFACEMETHODIMP BaseExplorerCommand::EnumSubCommands(_COM_Outptr_ IEnumExplorerCommand** enumCommands)
	{
		*enumCommands = nullptr;
		auto subCommands = SubCommands();
		bool hasSubCommands = subCommands != nullptr && subCommands.Size() > 0;
		if (hasSubCommands)
		{
			auto subMenu = winrt::make<SubMenu>(SubCommands());
			winrt::hresult result = subMenu.as(IID_PPV_ARGS(enumCommands));
			return result;
		}
		return E_NOTIMPL;
	}

	winrt::Windows::Foundation::Collections::IVectorView<winrt::ZipShellExt::BaseExplorerCommand> BaseExplorerCommand::SubCommands()
	{
		return nullptr;
	}

	/// <summary>
	/// SevenZipCommand ����
	/// </summary>
	SevenZipCommand::SevenZipCommand() {};

	IFACEMETHODIMP SevenZipCommand::GetTitle(_In_opt_ IShellItemArray* items, _Outptr_result_nullonfailure_ PWSTR* name)
	{
		// Ӧ���Ҽ��˵���ʾ�ı���
		winrt::hstring title = L"7zip";
		return SHStrDup(title.c_str(), name);
	}

	IFACEMETHODIMP SevenZipCommand::GetIcon(_In_opt_ IShellItemArray*, _Outptr_result_nullonfailure_ PWSTR* icon)
	{
		// Ӧ���Ҽ��˵���ʾ��ͼ��·������ҪICO��ʽ������Package.Current.InstalledLocation�ǵ�ǰӦ�õİ�װĿ¼
		winrt::hstring iconPath = winrt::Windows::ApplicationModel::Package::Current().InstalledLocation().Path() + L"\\Assets\\AppIcon.ico";
		return SHStrDup(iconPath.c_str(), icon);
	}

	IFACEMETHODIMP SevenZipCommand::GetState(_In_opt_ IShellItemArray* selection, _In_ BOOL okToBeSlow, _Out_ EXPCMDSTATE* cmdState)
	{
		// Ӧ���Ҽ��˵��������������״̬��Ϣ
		ExplorerCommandState state = ExplorerCommandState::Enabled;
		*cmdState = static_cast<EXPCMDSTATE>(state);

		return S_OK;
	}

	IFACEMETHODIMP SevenZipCommand::Invoke(_In_opt_ IShellItemArray* selection, _In_opt_ IBindCtx*)
	{
		// �û�����˲˵��󣬴������������IShellItemArray��������˵���ѡ����Ŀ����Ϣ��
		constexpr winrt::guid uuid = winrt::guid_of<SevenZipCommand>();
		
		return S_OK;
	}

	winrt::Windows::Foundation::Collections::IVectorView<winrt::ZipShellExt::BaseExplorerCommand> SevenZipCommand::SubCommands()
	{
		// ���б�����ò˵���������һ���˵�������
		return winrt::single_threaded_vector<winrt::ZipShellExt::BaseExplorerCommand>(
			{
				winrt::make<ExtractToCommand>(),
				winrt::make<AddTo7zCommand>(),
				winrt::make<AddToZipCommand>(),
				winrt::make<CompressAndEmailCommand>()
			}).GetView();
	}

	/// <summary>
	/// ExtractToCommand ����
	/// </summary>
	ExtractToCommand::ExtractToCommand() {};

	IFACEMETHODIMP ExtractToCommand::GetTitle(_In_opt_ IShellItemArray* items, _Outptr_result_nullonfailure_ PWSTR* name)
	{
		winrt::hstring title = L"Extract to ***";
		return SHStrDup(title.c_str(), name);
	}

	IFACEMETHODIMP ExtractToCommand::GetIcon(_In_opt_ IShellItemArray*, _Outptr_result_nullonfailure_ PWSTR* icon)
	{
		DWORD value = 0;
		DWORD size = sizeof(value);
		winrt::hstring iconPath = L"";
		if (const auto result = SHRegGetValueW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", L"AppsUseLightTheme", SRRF_RT_DWORD, nullptr, &value, &size); result == ERROR_SUCCESS && !!value)
		{
			iconPath = winrt::Windows::ApplicationModel::Package::Current().InstalledLocation().Path() + L"\\Assets\\unzip Light.ico";
		}
		else
		{
			iconPath = winrt::Windows::ApplicationModel::Package::Current().InstalledLocation().Path() + L"\\Assets\\unzip Dark.ico";
		}
		return SHStrDup(iconPath.c_str(), icon);
	}

	IFACEMETHODIMP ExtractToCommand::GetState(_In_opt_ IShellItemArray* selection, _In_ BOOL okToBeSlow, _Out_ EXPCMDSTATE* cmdState)
	{
		ExplorerCommandState state = ExplorerCommandState::Enabled;
		*cmdState = static_cast<EXPCMDSTATE>(state);
		return S_OK;
	}

	IFACEMETHODIMP ExtractToCommand::Invoke(_In_opt_ IShellItemArray* selection, _In_opt_ IBindCtx*)
	{
		constexpr winrt::guid uuid = winrt::guid_of<ExtractToCommand>();
		std::vector<std::wstring> FilePaths;

		//��ȡѡ��������ļ���·��
		//Get paths of all selected files
		if (selection) 
		{
			DWORD Count = 0;
			if (SUCCEEDED(selection->GetCount(&Count)))
			{
				for (DWORD i = 0; i < Count; ++i)
				{
					winrt::com_ptr<IShellItem> Item;
					if (SUCCEEDED(selection->GetItemAt(i, Item.put())))
					{
						LPWSTR DisplayName = nullptr;
						if (SUCCEEDED(Item->GetDisplayName(
							SIGDN_FILESYSPATH,
							&DisplayName)))
						{
							FilePaths.push_back(std::wstring(DisplayName));
							::CoTaskMemFree(DisplayName);
						}
					}
				}
			}
		}


		//��ӡ����ѡ���ļ���·�����뿴����ô����֧������
		winrt::hstring tempFilePath = Windows::Storage::ApplicationData::Current().LocalCacheFolder().Path();
		std::string outputFilePath = to_string(tempFilePath) + std::string("\\ExtractPathsTempFile.out");
		std::string tmp;

		std::wstring_convert<std::codecvt_utf8<wchar_t>> conv; //����utf8ת����

		std::ofstream destFile(outputFilePath, std::ios::out);

		destFile << FilePaths.size() << std::endl; //��һ������ļ��������ڶ�ȡ

		for (int i = 0;i < FilePaths.size(); ++i)
		{
			tmp = conv.to_bytes(FilePaths[i]);
			destFile << tmp << std::endl;
		}
		//MessageBox(NULL, tmp, NULL, MB_OK);
		destFile.close();



		std::wstring appName = L"";
		std::wstring commandLineStr = appName + L"7Zip.App.exe -extract";
		//����Ŀǰֻѡȡ�˵�һ���ļ���·�������������Ӵ���

		LPWSTR cmdLine = StrDupW(commandLineStr.c_str());

		STARTUPINFO si;
		PROCESS_INFORMATION pi;
		ZeroMemory(&si, sizeof(si));
		ZeroMemory(&pi, sizeof(pi));

		CreateProcessW(NULL, cmdLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
		int lastErr = GetLastError();
		
		return S_OK;
	}

	winrt::Windows::Foundation::Collections::IVectorView<winrt::ZipShellExt::BaseExplorerCommand> ExtractToCommand::SubCommands()
	{
		return nullptr;
	}

	/// <summary>
	/// AddTo7zCommand ����
	/// </summary>
	AddTo7zCommand::AddTo7zCommand() {};

	IFACEMETHODIMP AddTo7zCommand::GetTitle(_In_opt_ IShellItemArray* items, _Outptr_result_nullonfailure_ PWSTR* name)
	{
		winrt::hstring title = L"Add to ***.7z";
		return SHStrDup(title.c_str(), name);
	}

	IFACEMETHODIMP AddTo7zCommand::GetIcon(_In_opt_ IShellItemArray*, _Outptr_result_nullonfailure_ PWSTR* icon)
	{
		DWORD value = 0;
		DWORD size = sizeof(value);
		winrt::hstring iconPath = L"";
		if (const auto result = SHRegGetValueW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", L"AppsUseLightTheme", SRRF_RT_DWORD, nullptr, &value, &size); result == ERROR_SUCCESS && !!value)
		{
			iconPath = winrt::Windows::ApplicationModel::Package::Current().InstalledLocation().Path() + L"\\Assets\\zip Light.ico";
		}
		else
		{
			iconPath = winrt::Windows::ApplicationModel::Package::Current().InstalledLocation().Path() + L"\\Assets\\zip Dark.ico";
		}
		return SHStrDup(iconPath.c_str(), icon);
	}

	IFACEMETHODIMP AddTo7zCommand::GetState(_In_opt_ IShellItemArray* selection, _In_ BOOL okToBeSlow, _Out_ EXPCMDSTATE* cmdState)
	{
		ExplorerCommandState state = ExplorerCommandState::Enabled;
		*cmdState = static_cast<EXPCMDSTATE>(state);
		return S_OK;
	}

	IFACEMETHODIMP AddTo7zCommand::Invoke(_In_opt_ IShellItemArray* selection, _In_opt_ IBindCtx*)
	{
		constexpr winrt::guid uuid = winrt::guid_of<AddTo7zCommand>();
		std::vector<std::wstring> FilePaths;

		if (selection)
		{
			DWORD Count = 0;
			if (SUCCEEDED(selection->GetCount(&Count)))
			{
				for (DWORD i = 0; i < Count; ++i)
				{
					winrt::com_ptr<IShellItem> Item;
					if (SUCCEEDED(selection->GetItemAt(i, Item.put())))
					{
						LPWSTR DisplayName = nullptr;
						if (SUCCEEDED(Item->GetDisplayName(
							SIGDN_FILESYSPATH,
							&DisplayName)))
						{
							FilePaths.push_back(std::wstring(DisplayName));
							::CoTaskMemFree(DisplayName);
						}
					}
				}
			}
		}

		std::wstring appName = L"7Zip.App.exe";
		std::wstring commandLineStr = appName + L" -compress " + L"-7z "+ FilePaths.at(0);
		//����Ŀǰֻѡȡ�˵�һ���ļ���·�������������Ӵ���
		LPWSTR cmdLine = StrDupW(commandLineStr.c_str());

		STARTUPINFO si;
		PROCESS_INFORMATION pi;
		ZeroMemory(&si, sizeof(si));
		ZeroMemory(&pi, sizeof(pi));

		CreateProcessW(NULL, cmdLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
		int lastErr = GetLastError();

		return S_OK;
	}

	winrt::Windows::Foundation::Collections::IVectorView<winrt::ZipShellExt::BaseExplorerCommand> AddTo7zCommand::SubCommands()
	{
		return nullptr;
	}

	/// <summary>
	/// AddToZipCommand ����
	/// </summary>
	AddToZipCommand::AddToZipCommand() {};

	IFACEMETHODIMP AddToZipCommand::GetTitle(_In_opt_ IShellItemArray* items, _Outptr_result_nullonfailure_ PWSTR* name)
	{
		winrt::hstring title = L"Add to ***.zip";
		return SHStrDup(title.c_str(), name);
	}

	IFACEMETHODIMP AddToZipCommand::GetIcon(_In_opt_ IShellItemArray*, _Outptr_result_nullonfailure_ PWSTR* icon)
	{
		DWORD value = 0;
		DWORD size = sizeof(value);
		winrt::hstring iconPath = L"";
		if (const auto result = SHRegGetValueW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", L"AppsUseLightTheme", SRRF_RT_DWORD, nullptr, &value, &size); result == ERROR_SUCCESS && !!value) 
		{
			iconPath = winrt::Windows::ApplicationModel::Package::Current().InstalledLocation().Path() + L"\\Assets\\zip Light.ico";
		}
		else 
		{
			iconPath = winrt::Windows::ApplicationModel::Package::Current().InstalledLocation().Path() + L"\\Assets\\zip Dark.ico";
		}
		
		return SHStrDup(iconPath.c_str(), icon);
	}

	IFACEMETHODIMP AddToZipCommand::GetState(_In_opt_ IShellItemArray* selection, _In_ BOOL okToBeSlow, _Out_ EXPCMDSTATE* cmdState)
	{
		ExplorerCommandState state = ExplorerCommandState::Enabled;
		*cmdState = static_cast<EXPCMDSTATE>(state);
		return S_OK;
	}

	IFACEMETHODIMP AddToZipCommand::Invoke(_In_opt_ IShellItemArray* selection, _In_opt_ IBindCtx*)
	{
		constexpr winrt::guid uuid = winrt::guid_of<AddToZipCommand>();
		return S_OK;
	}

	winrt::Windows::Foundation::Collections::IVectorView<winrt::ZipShellExt::BaseExplorerCommand> AddToZipCommand::SubCommands()
	{
		return nullptr;
	}

	/// <summary>
	/// CompressAndEmailCommand ����
	/// </summary>
	CompressAndEmailCommand::CompressAndEmailCommand() {};

	IFACEMETHODIMP CompressAndEmailCommand::GetTitle(_In_opt_ IShellItemArray* items, _Outptr_result_nullonfailure_ PWSTR* name)
	{
		winrt::hstring title = L"Compress and email";
		return SHStrDup(title.c_str(), name);
	}

	IFACEMETHODIMP CompressAndEmailCommand::GetIcon(_In_opt_ IShellItemArray*, _Outptr_result_nullonfailure_ PWSTR* icon)
	{
		winrt::hstring iconPath = L"";
		return SHStrDup(iconPath.c_str(), icon);
	}

	IFACEMETHODIMP CompressAndEmailCommand::GetState(_In_opt_ IShellItemArray* selection, _In_ BOOL okToBeSlow, _Out_ EXPCMDSTATE* cmdState)
	{
		ExplorerCommandState state = ExplorerCommandState::Enabled;
		*cmdState = static_cast<EXPCMDSTATE>(state);
		return S_OK;
	}

	IFACEMETHODIMP CompressAndEmailCommand::Invoke(_In_opt_ IShellItemArray* selection, _In_opt_ IBindCtx*)
	{
		constexpr winrt::guid uuid = winrt::guid_of<CompressAndEmailCommand>();
		return S_OK;
	}

	winrt::Windows::Foundation::Collections::IVectorView<winrt::ZipShellExt::BaseExplorerCommand> CompressAndEmailCommand::SubCommands()
	{
		return nullptr;
	}
}