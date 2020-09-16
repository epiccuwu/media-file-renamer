#include <filesystem>
#include <array>
#include <string_view>

#include "winlamb/dialog_main.h"
#include "winlamb/button.h"
#include "winlamb/textbox.h"
#include "winlamb/sysdlg.h"
#include "winlamb/listview.h"
#include "resource.h"

class c_window : public wl::dialog_main {
	wl::textbox m_folder_path;
	wl::textbox m_series_name;
	wl::listview m_files_in_path;
	wl::button m_browse;
	wl::button m_rename;

	std::array< std::wstring, 7 > m_icons = { L".exe", L".mp4", L".mkv", L".txt", L".gif", L".avi", L".mov" };
public:
	auto update_list( std::wstring_view folder ) {
		const auto get_icon_index = [&]( std::wstring_view extention ) {
			for ( auto i = m_icons.begin( ); i != m_icons.end( ); i++ ) {
				if ( i->find( extention ) != std::wstring::npos ) {
					return std::distance( i, m_icons.end( ) ) - 1;
				}
			}
		};

		m_files_in_path.items.remove_all( );
		for ( const auto& dir_entry : std::filesystem::recursive_directory_iterator( folder ) ) {
			if ( !dir_entry.exists( ) ) {
				continue;
			}

			const auto path = dir_entry.path( );
			if ( path.empty( ) ) {
				continue;
			}

			const auto extention = path.extension( ).wstring( );
			if ( extention.empty( ) ) {
				continue;
			}

			m_files_in_path.items.add_with_icon( path.wstring( ), get_icon_index( extention ) );
		}
	}

	c_window( ) {
		setup.dialogId = IDD_MAIN_DIALOG;

		on_message( WM_INITDIALOG, [&]( wl::params p ) -> LRESULT {
			m_folder_path.assign( this, IDC_CURRENT_FOLDER );
			m_series_name.assign( this, IDC_SERIES_NAME );
			m_browse.assign( this, IDC_BROWSE );
			m_rename.assign( this, IDC_APPLY );
			m_files_in_path.assign( this, IDC_FILE_LIST ).columns.add( L"File Name:", 450 );
			m_files_in_path.imageList16.load_from_shell( { L"exe", L"mp4", L"mkv", L"txt", L"gif", L"avi", L"mov" } );
			return TRUE;
		} );

		on_command( IDC_BROWSE, [&]( wl::params p ) -> LRESULT {
			std::wstring folder;
			wl::sysdlg::choose_folder( hwnd( ), folder );

			if ( !folder.empty( ) ) {
				m_folder_path.set_text( folder );
				update_list( folder );
			}

			return TRUE;
		} );

		on_command( IDC_APPLY, [&]( wl::params p ) -> LRESULT {
			const auto folder = std::filesystem::path( m_folder_path.get_text( ) ).concat( "\\" );
			if ( !folder.empty( ) ) {
				auto episode = 1;
				for ( const auto& dir_entry : std::filesystem::recursive_directory_iterator( folder ) ) {
					if ( !dir_entry.exists( ) || dir_entry.is_directory( ) ) {
						continue;
					}

					const auto path = dir_entry.path( );
					if ( path.empty( ) || path.filename( ).empty( ) || !path.has_extension( ) ) {
						continue;
					}

					auto final_name = m_series_name.get_text( );
					if ( final_name.empty( ) ) {
						final_name = L"unknown";
					}

					final_name += L"-e";
					final_name += std::to_wstring( episode++ );
					final_name += path.extension( ).wstring( );

					std::filesystem::rename( path, folder.wstring( ) + final_name );
				}

				update_list( folder.wstring( ) );

				std::wstring msg = L"renamed a total of ";
				msg.append( std::to_wstring( episode - 1 ) ).append( L" files." );
				wl::sysdlg::msgbox( this, L"media-renamer", msg, MB_OK );
			}
			return TRUE;
		} );
	}
};

RUN( c_window )