import { Component, OnInit, ViewEncapsulation } from '@angular/core';
import { Utility } from '@app/common/Utility';
import { ControllerService } from '@app/services/controller.service';
import {Animations} from '@app/animations';
import { UIRolePermissions } from '@sdk/v1/models/generated/UI-permissions-enum';
import { UIConfigsService } from '@app/services/uiconfigs.service';

@Component({
  selector: 'app-updatecluster-tls',
  templateUrl: './updatecluster-tls.component.html',
  encapsulation: ViewEncapsulation.None,
  animations: [Animations],
  styleUrls: ['./updatecluster-tls.component.scss']
})
export class UpdateclusterTlsComponent implements OnInit {
  certMode = false;
  constructor(protected _controllerService: ControllerService,
    ) { }
  ngOnInit() {
   let buttons = [];
      buttons = [{
        cssClass: 'global-button-primary updateclustertls-toolbar-button',
        text: 'UPDATE',
        callback: () => { this.certMode =  !this.certMode; },
      }];
    this._controllerService.setToolbarData({
      buttons: buttons,
      breadcrumb: [{ label: 'Update Server Certificate', url: Utility.getBaseUIUrl() + 'admin/certificate' }],
    });
  }
  onCertificateCancel(event) {
    this.certMode = false;
  }

}
