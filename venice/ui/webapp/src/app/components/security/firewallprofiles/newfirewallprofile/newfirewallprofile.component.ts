import { AfterViewInit, ChangeDetectionStrategy, Component, OnDestroy, OnInit, ViewEncapsulation } from '@angular/core';
import { CreationForm } from '@app/components/shared/tableviewedit/tableviewedit.component';
import { ControllerService } from '@app/services/controller.service';
import { SecurityService } from '@app/services/generated/security.service';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { ISecurityFirewallProfile, SecurityFirewallProfile } from '@sdk/v1/models/generated/security';
import { minValueValidator } from '@sdk/v1/utils/validators';

/**
 * As FirewallProfile object is a singleton object within Venice.
 * We make the create-form to perform object update.
 *
 * These API and properties tweak system to perform object update.
 *
 * isSingleton: boolean = true;
 * postNgInit()
 */
@Component({
  selector: 'app-newfirewallprofile',
  templateUrl: './newfirewallprofile.component.html',
  styleUrls: ['./newfirewallprofile.component.scss'],
  encapsulation: ViewEncapsulation.None,
  changeDetection: ChangeDetectionStrategy.OnPush,
})
export class NewfirewallprofileComponent extends CreationForm<SecurityFirewallProfile, SecurityFirewallProfile> implements OnInit, AfterViewInit, OnDestroy {

  isSingleton: boolean = true;
  saveButtonTooltip: string = 'There is no change';

  constructor(protected _controllerService: ControllerService,
    protected uiconfigsService: UIConfigsService,
    protected securityService: SecurityService,
  ) {
    super(_controllerService, uiconfigsService, SecurityFirewallProfile);
  }

  getClassName(): string {
    return this.constructor.name;
  }


  setToolbar(): void {
    const currToolbar = this._controllerService.getToolbarData();
    this.oldButtons = currToolbar.buttons;
    currToolbar.buttons = [
      {
        cssClass: 'global-button-primary newfirewallprofile-button newfirewallprofile-button-SAVE',
        text: 'SAVE FIREWALL PROFILE',
        callback: () => {
          this.saveObject();  // should invoke updateObject() in super class as this.isSingleton is true
        },
        computeClass: () => this.computeButtonClass(),
        genTooltip: () => this.getTooltip(),
      },
      {
        cssClass: 'global-button-neutral newfirewallprofile-button newfirewallprofile-button-CANCEL',
        text: 'CANCEL',
        callback: () => {
          this.cancelObject();
        }
      },
    ];

    this._controllerService.setToolbarData(currToolbar);

  }

  /**
   * compute whether to enable [save] button
  */
  computeButtonClass() {
    let enable = false;
    if (!this.newObject.$formGroup.dirty) {
      // if no change, don't enable it
      enable = false;
    } else {
      if (this.isFormValid()) {
        enable = true;
      }
    }

    if (enable) {
      return '';
    }
    return 'global-button-disabled';
  }

  getTooltip(): string {
    return this.saveButtonTooltip;
  }

  createObject(object: SecurityFirewallProfile) {
    return null;
  }

  // tcp,icmp,udp, any + 1< x < 255
  updateObject(newObject: ISecurityFirewallProfile, oldObject: ISecurityFirewallProfile) {
    return this.securityService.UpdateFirewallProfile(oldObject.meta.name, newObject, null, oldObject, true, false);
  }

  postNgInit() {
    // use selectedFirewallProfile to populate this.newObject
    this.newObject = new SecurityFirewallProfile(this.objectData);
    this.newObject.$formGroup.get(['meta', 'name']).disable();

    this.addFieldValidator(this.newObject.$formGroup.get(['spec', 'session-idle-timeout']), this.isTimeoutValid('session-idle-timeout'));
    this.addFieldValidator(this.newObject.$formGroup.get(['spec', 'tcp-connection-setup-timeout']), this.isTimeoutValid('tcp-connection-setup-timeout'));
    this.addFieldValidator(this.newObject.$formGroup.get(['spec', 'tcp-close-timeout']), this.isTimeoutValid('tcp-close-timeout'));
    this.addFieldValidator(this.newObject.$formGroup.get(['spec', 'tcp-half-closed-timeout']), this.isTimeoutValid('tcp-half-closed-timeout'));

    this.addFieldValidator(this.newObject.$formGroup.get(['spec', 'tcp-drop-timeout']), this.isTimeoutValid('tcp-drop-timeout'));
    this.addFieldValidator(this.newObject.$formGroup.get(['spec', 'udp-drop-timeout']), this.isTimeoutValid('udp-drop-timeout'));
    this.addFieldValidator(this.newObject.$formGroup.get(['spec', 'icmp-drop-timeout']), this.isTimeoutValid('icmp-drop-timeout'));
    this.addFieldValidator(this.newObject.$formGroup.get(['spec', 'drop-timeout']), this.isTimeoutValid('drop-timeout'));

    this.addFieldValidator(this.newObject.$formGroup.get(['spec', 'tcp-timeout']), this.isTimeoutValid('tcp-timeout'));
    this.addFieldValidator(this.newObject.$formGroup.get(['spec', 'udp-timeout']), this.isTimeoutValid('udp-timeout'));
    this.addFieldValidator(this.newObject.$formGroup.get(['spec', 'icmp-timeout']), this.isTimeoutValid('icmp-timeout'));

    // add minValueValidator
    this.addFieldValidator(this.newObject.$formGroup.get(['spec', 'tcp-half-open-session-limit']), minValueValidator(0));
    this.addFieldValidator(this.newObject.$formGroup.get(['spec', 'udp-active-session-limit']), minValueValidator(0));
    this.addFieldValidator(this.newObject.$formGroup.get(['spec', 'icmp-active-session-limit']), minValueValidator(0));
    this.addFieldValidator(this.newObject.$formGroup.get(['spec', 'other-active-session-limit']), minValueValidator(0));

  }




  generateUpdateSuccessMsg(object: SecurityFirewallProfile): string {
    return 'Updated firewall profile ' + object.meta.name;
  }

  generateCreateSuccessMsg(object: SecurityFirewallProfile): string {
    throw new Error('Firewall Profile is a singleton object. Create method is not supported');
  }

  isFormValid(): boolean {
    this.saveButtonTooltip = (!this.newObject.$formGroup.valid) ? 'There are validation errors.' : 'Save';
    return this.newObject.$formGroup.valid;
  }

}
