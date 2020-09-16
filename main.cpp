#include <filesystem>
#include <array>

#include "winlamb/dialog_main.h"
#include "winlamb/button.h"
#include "winlamb/textbox.h"
#include "winlamb/sysdlg.h"
#include "winlamb/file.h"
#include "winlamb/listview.h"
#include "resource.h"

class CWindow : public wl::dialog_main {
	wl::textbox m_folder_path;
	wl::textbox m_series_name;
	wl::listview m_files_in_path;
	wl::button m_browse;
	wl::button m_rename;

	std::array< std::wstring, 7 > m_icons = { L"exe", L"mp4", L"mkv", L"txt", L"gif", L"avi", L"mov" };
public:
	CWindow( ) {
		setup.dialogId = IDD_MAIN_DIALOG;

		on_message( WM_INITDIALOG, [&]( wl::params p ) -> LRESULT {
			m_folder_path.assign( this, IDC_CURRENT_FOLDER );
			m_series_name.assign( this, IDC_SERIES_NAME );
			m_browse.assign( this, IDC_BROWSE );
			m_rename.assign( this, IDC_APPLY );
			m_files_in_path.assign( this, IDC_FILE_LIST )
				.columns.add( L"File Name:", 450 );

			m_files_in_path.imageList16.load_from_shell( { L"exe", L"mp4", L"mkv", L"txt", L"gif", L"avi", L"mov" } );
			return TRUE;
		} );

		on_command( IDC_BROWSE, [&]( wl::params p ) -> LRESULT {
			std::wstring folder;
			wl::sysdlg::choose_folder( hwnd( ), folder );

			if ( !folder.empty( ) ) {
				m_folder_path.set_text( folder );
				m_files_in_path.items.remove_all( );
				for ( const auto& dir_entry : std::filesystem::recursive_directory_iterator( folder ) ) {
					if ( !dir_entry.exists( ) || dir_entry.is_directory( ) ) {
						continue;
					}

					auto path = dir_entry.path( );
					if ( path.empty( ) ) {
						continue;
					}

					auto extention = path.extension( ).wstring( );
					if ( extention.empty( ) ) {
						continue;
					}

					extention.erase( extention.begin( ) );
					std::uint32_t index = 0;
					for ( auto i = m_icons.begin( ); i != m_icons.end( ); i++ ) {
						if ( i->find( extention ) != std::wstring::npos ) {
							index = std::distance( i, m_icons.end( ) ) - 1;
						}
					}

					m_files_in_path.items.add_with_icon( path.wstring( ), index );
				}
			}

			return TRUE;
		} );

		on_command( IDC_APPLY, [&]( wl::params p ) -> LRESULT {
			auto folder = std::filesystem::path( m_folder_path.get_text( ) );
			if ( !folder.empty( ) ) {
				folder.concat( L"\\" );
				int episode = 1;
				for ( const auto& dir_entry : std::filesystem::recursive_directory_iterator( folder ) ) {
					if ( !dir_entry.exists( ) ) {
						continue;
					}

					auto old_path = dir_entry.path( );
					auto old_file_name = old_path.filename( ).wstring( );
					auto extention = old_path.extension( ).wstring( );
					auto new_dir = std::filesystem::path( m_folder_path.get_text( ) );

					auto final_name = m_series_name.get_text( );
					if ( final_name.empty( ) ) {
						final_name = L"unknown";
					}

					final_name += L"e";
					final_name += std::to_wstring( episode++ );
					final_name += extention;

					std::filesystem::rename( old_path, new_dir.concat( final_name ) );
				}
			}
			return TRUE;
		} );
	}
};

RUN( CWindow )