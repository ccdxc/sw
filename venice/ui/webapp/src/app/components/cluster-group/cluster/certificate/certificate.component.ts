import {AfterViewInit, Component, OnDestroy, OnInit, ViewEncapsulation, EventEmitter, Output} from '@angular/core';
import {BaseComponent} from '@components/base/base.component';
import {ClusterService} from '@app/services/generated/cluster.service';
import {ControllerService} from '@app/services/controller.service';
import {IClusterUpdateTLSConfigRequest, ClusterUpdateTLSConfigRequest} from '@sdk/v1/models/generated/cluster';
import {Animations} from '@app/animations';
import {required} from '@sdk/v1/utils/validators';
import {Utility} from '@common/Utility';


@Component({
  selector: 'app-certificate',
  templateUrl: './certificate.component.html',
  styleUrls: ['./certificate.component.scss'],
  animations: [Animations],
  encapsulation: ViewEncapsulation.None,
})
export class CertificateComponent extends BaseComponent implements OnInit, OnDestroy {
  newCert: ClusterUpdateTLSConfigRequest;
  subscriptions = [];
  oldToolbarData;
  @Output() cancelEvent: EventEmitter<any> = new EventEmitter();

  constructor(private clusterService: ClusterService,
              protected controllerService: ControllerService, ) {
    super(controllerService);
  }

  ngOnInit() {
    // store old toolbar data
    this.oldToolbarData = this.controllerService.getToolbarData();
    const newToolbarData = this.controllerService.getToolbarData();
    newToolbarData.buttons = [
      {
        cssClass: 'global-button-primary certificate-toolbar-button',
        text: 'UPLOAD',
        callback: () => {
          this.upload();
        },
        computeClass: () => this.isAllInputsValidated() ? '' : 'global-button-disabled',
      },
      {
        cssClass: 'global-button-neutral certificate-toolbar-button',
        text: 'CANCEL',
        callback: () => {
          this.cancel();
        },
      },
    ];
    this.controllerService.setToolbarData(newToolbarData);
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
    const sub = this.clusterService.UpdateTLSConfig(cert).subscribe(response => {
        this.controllerService.invokeSuccessToaster(Utility.UPDATE_SUCCESS_SUMMARY, 'Successfully updated TLS configuration');
        this.cancel();
      },
      this.controllerService.restErrorHandler(Utility.UPDATE_FAILED_SUMMARY)
    );
    this.subscriptions.push(sub);
  }

  cancel() {
    this.cancelEvent.emit(false);
    this.controllerService.setToolbarData(this.oldToolbarData);
  }

  ngOnDestroy() {
    this.subscriptions.forEach((subscription) => {
      subscription.unsubscribe();
    });
  }

}
