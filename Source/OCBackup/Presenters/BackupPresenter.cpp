#include "BackupPresenter.h"

#include <QFileDialog>
#include <QTreeView>
#include <QDebug>
#include <QFileInfoList>
#include <QStorageInfo>

#include <DriveManager.h>

#include "Services/DriveTransferService.h"

#include <Controls/ProgressDialog.h>

#include <Events/EventBus.h>
#include "Log/Logger.h"
#include "Task/TaskManager.h"
#include "Transfer/DriveTransfer.h"

BackupPresenter::BackupPresenter(IBackupView& view, OCEventBus* eventBus) : BasePresenter(eventBus),
                                                                            _view(&view),
                                                                            _currentDrive(0)
{
	_driveManager = new DriveManager();

	SetupSignals();

	_driveManager->RequestDriveList();
}

void BackupPresenter::SetupSignals() const
{
	connect(_driveManager, SIGNAL(DriveListChanged(std::vector<PathInfo>)), this, SLOT(DriveListChanged(std::vector<PathInfo>)));
	connect(_view, SIGNAL(DriveSelectionChanged(int)), this, SLOT(DriveSelectionChanged(int)));

	connect(_view, &IBackupView::AddDestinationClicked, this, &BackupPresenter::AddDestination);
	connect(_view, &IBackupView::FolderSelectionChanged, this, &BackupPresenter::FolderSelectionChanged);
	connect(_view, &IBackupView::StartTransfer, this, &BackupPresenter::StartTransfer);
}

void BackupPresenter::receive(const OCEvent& event) const
{
	int j = 0;

	j++;

	int c = j;

	OC_LOG_INFO("BackupPresenter received EventA -> 1");
}


void receive2(const OCEvent& event)
{
	int j = 0;

	j++;

	int c = j;

	OC_LOG_INFO("BackupPresenter received EventA -> 2");
}

void BackupPresenter::StartTransfer()
{
	StartDriveTransferEvent transferEvent;
	
	if(_driveList.empty())
	{
		return;
	}

	transferEvent.SetSourcePath(_driveList[_currentDrive].DrivePath);
	std::vector<std::string> destinationPaths;

	for (PathInfo destination : _destinationList)
	{
		destinationPaths.push_back(destination.Path);
	}

	transferEvent.SetDestinationsPaths(destinationPaths);
	//GetEventBus()->FireEvent<StartDriveTransferEvent>(transferEvent);

	ProgressDialog* progressDialog = new ProgressDialog();
	progressDialog->show();
}

void BackupPresenter::DriveListChanged(std::vector<PathInfo> driveList)
{
	_driveList = driveList;
	_view->SetDriveList(driveList);

	if (!_driveList.empty())
	{
		QString drive = QString::fromStdString(driveList.at(0).DrivePath);
		_view->SetCurrentFolder(drive);

		FolderSelectionChanged(drive);
		/*std::vector<std::string> flist;
		flist.push_back("C:/Temp/test.jpg");
		_view->SetItemList(flist);*/
	}
	else
	{
		_view->SetCurrentFolder("");
	}
}

void BackupPresenter::DriveSelectionChanged(int driveIndex)
{
	if (_driveList.empty())
	{
		return;
	}

	QString folderPath = QString::fromStdString(_driveList.at(driveIndex).DrivePath);
	_view->SetCurrentFolder(folderPath);

	_currentDrive = driveIndex;

	FolderSelectionChanged(folderPath);
}

void BackupPresenter::AddDestination()
{
	QFileDialog dialog;
	dialog.setFileMode(QFileDialog::Directory);
	dialog.setOption(QFileDialog::ShowDirsOnly);
	dialog.setViewMode(QFileDialog::Detail);
	int result = dialog.exec();

	if (result)
	{
		QStorageInfo storage(dialog.selectedFiles()[0]);

		PathInfo pathInfo;

		QString path = dialog.directory().path();
		
		pathInfo.RelativePath = path.right(path.length() - storage.rootPath().length()).toStdString();
		if (pathInfo.RelativePath == "")
		{
			pathInfo.RelativePath = "/";
		}

		pathInfo.Path = path.toStdString();
		pathInfo.DriveName = storage.displayName().toStdString();
		pathInfo.DrivePath = storage.rootPath().toStdString();

		_destinationList.push_back(pathInfo);
		_view->SetDestinationList(_destinationList);
	}
}

void BackupPresenter::FolderSelectionChanged(QString folderPath) const
{
	QDir dir(folderPath);
	QFileInfoList fileList = dir.entryInfoList(QDir::NoDotAndDotDot | QDir::Files);

	std::vector<FileInfo> fileInfoList;

	for (QFileInfo fileInfo : fileList)
	{
		FileInfo info(fileInfo.path(), fileInfo.fileName(), fileInfo.size());
		fileInfoList.push_back(info);
	}

	_view->SetItemList(fileInfoList);
}
