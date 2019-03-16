import { Component, OnInit, ViewEncapsulation } from '@angular/core';
import { Animations } from '@app/animations';
import { ControllerService } from '@app/services/controller.service';
import { BaseComponent } from '@app/components/base/base.component';
import { Icon } from '@app/models/frontend/shared/icon.interface';

@Component({
  selector: 'app-systemupgrade',
  templateUrl: './systemupgrade.component.html',
  styleUrls: ['./systemupgrade.component.scss'],
  animations: [Animations],
  encapsulation: ViewEncapsulation.None
})
export class SystemupgradeComponent extends BaseComponent implements OnInit {

  uploadedFiles: any[] = [];

  bodyIcon: Icon = {
    margin: {
      top: '8px',
      left: '10px',
    },
    matIcon: 'cloud_upload'
  };

  constructor(protected _controllerService: ControllerService
  ) {
    super(_controllerService);
  }

  ngOnInit() {
    this.setToolbarItems();
  }

  protected setToolbarItems() {
    this._controllerService.setToolbarData({
      buttons: [
      ],
      breadcrumb: [{ label: 'System Upgrade', url: '' }, { label: 'File Upload', url: '' }]
    });
  }

  /**
   * This API serves html template
   */
  onUpload(event) {
    const files = event.files;
    const filenames = this.getFilesNames(files);
    this._controllerService.invokeInfoToaster('Upload succeeded', 'Uploaded files: ' + filenames.join(',') + '. System will validate uploaded files.');
  }

   /**
   * This API serves html template
   */
  onError(event) {
    const files = event.files;
    const filenames = this.getFilesNames(files);
    this._controllerService.invokeErrorToaster('Upload failed', 'Involved files: ' + filenames.join(','));
  }

  getFilesNames(files: any[]): string[] {
    const filenames: string [] = [];
    for (const  file of files) {
      this.uploadedFiles.push(file);
      filenames.push(file.name);
    }
    return filenames;
  }

}
