import { Component, OnDestroy, OnInit, ViewEncapsulation} from '@angular/core';
import { Animations } from '@app/animations';

import { ClusterUpdateTLSConfigRequest, IClusterUpdateTLSConfigRequest } from '@sdk/v1/models/generated/cluster';
import {required} from '@sdk/v1/utils/validators';

import { Utility } from '@app/common/Utility';
import { BaseComponent } from '@app/components/base/base.component';
import { Eventtypes } from '@app/enum/eventtypes.enum';
import { ControllerService } from '@app/services/controller.service';
import { ClusterService } from '@app/services/generated/cluster.service';

@Component({
  selector: 'app-updatecluster-tls',
  templateUrl: './updatecluster-tls.component.html',
  encapsulation: ViewEncapsulation.None,
  animations: [Animations],
  styleUrls: ['./updatecluster-tls.component.scss']
})
export class UpdateclusterTlsComponent extends BaseComponent implements OnInit, OnDestroy {
  newCert: ClusterUpdateTLSConfigRequest;
  subscriptions = [];

  constructor(private clusterService: ClusterService, protected controllerService: ControllerService) {
    super(controllerService);
  }

  ngOnInit() {
    this.controllerService.publish(Eventtypes.COMPONENT_INIT, { 'component': 'UpdateclusterTlsComponent', 'state': Eventtypes.COMPONENT_INIT });
    this.controllerService.setToolbarData({
      breadcrumb: [{ label: 'Update Server Certificate', url: Utility.getBaseUIUrl() + 'admin/certificate' }],
      buttons: [
        {
          cssClass: 'global-button-primary updateclustertlsCert-toolbar-button',
          text: 'UPLOAD',
          callback: () => {
            this.upload();
          },
          computeClass: () => this.isAllInputsValidated() ? '' : 'global-button-disabled',
        },
        {
          cssClass: 'global-button-neutral updateclustertlsCert-toolbar-button',
          text: 'CLEAR FORM',
          callback: () => {
            this.cancel();
          },
        },
      ]
    });
    this.newCert = new ClusterUpdateTLSConfigRequest();
    // must not be empty for key and certs
    // key is not required.
    this.newCert.$formGroup.get(['certs']).setValidators([required]);
  }

  isAllInputsValidated() {
    const hasFormGroupError = Utility.getAllFormgroupErrors(this.newCert.$formGroup);
    return hasFormGroupError === null;
  }

  upload() {
    const cert: IClusterUpdateTLSConfigRequest = this.newCert.getFormGroupValues();
    cert.key = cert.key.replace(/\\n/g, '\n');
    cert.certs = cert.certs.replace(/\\n/g, '\n');
    const sub = this.clusterService.UpdateTLSConfig(cert).subscribe(() => {
        this.controllerService.invokeSuccessToaster(Utility.UPDATE_SUCCESS_SUMMARY, 'Successfully updated TLS configuration');
        this.cancel();
      },
      this.controllerService.restErrorHandler(Utility.UPDATE_FAILED_SUMMARY)
    );
    this.subscriptions.push(sub);
  }

  cancel() {
    this.newCert.$formGroup.reset();
  }

  ngOnDestroy() {
    this.subscriptions.forEach((subscription) => {
      subscription.unsubscribe();
    });
    this.controllerService.publish(Eventtypes.COMPONENT_INIT, { 'component': 'UpdateclusterTlsComponent', 'state': Eventtypes.COMPONENT_DESTROY });
  }
}
