import { ComponentFixture, TestBed } from '@angular/core/testing';
import { configureTestSuite } from 'ng-bullet';
import { NewflowexportpolicyComponent } from './newflowexportpolicy.component';
import { TroubleshootGroupModule } from '@app/components/troubleshoot-group/troubleshoot-group.module';
import { SharedModule } from '@app/components/shared/shared.module';
import { ReactiveFormsModule, FormsModule } from '@angular/forms';
import { ControllerService } from '@app/services/controller.service';
import { ConfirmationService } from 'primeng/primeng';
import { LogService } from '@app/services/logging/log.service';
import { LogPublishersService } from '@app/services/logging/log-publishers.service';
import { MatIconRegistry } from '@angular/material';
import { MonitoringService } from '@app/services/generated/monitoring.service';
import { MessageService } from '@app/services/message.service';
import { RouterTestingModule } from '@angular/router/testing';
import { HttpClientTestingModule } from '@angular/common/http/testing';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { AuthService } from '@app/services/auth.service';
import { PrimengModule } from '@app/lib/primeng.module';
import { WidgetsModule } from 'web-app-framework';
import { MaterialdesignModule } from '@app/lib/materialdesign.module';
import { SecurityService } from '@app/services/generated/security.service';
import { Utility } from '@app/common/Utility';


describe('NewflowexportComponent', () => {
  let component: NewflowexportpolicyComponent;
  let fixture: ComponentFixture<NewflowexportpolicyComponent>;

  configureTestSuite(() => {
    TestBed.configureTestingModule({
      declarations: [NewflowexportpolicyComponent],
      imports: [
        TroubleshootGroupModule,
        SharedModule,
        ReactiveFormsModule,
        RouterTestingModule,
        HttpClientTestingModule,
        FormsModule,
        NoopAnimationsModule,
        PrimengModule,
        WidgetsModule,
        MaterialdesignModule,
      ],
      providers: [
        ControllerService,
        UIConfigsService,
        AuthService,
        ConfirmationService,
        LogService,
        LogPublishersService,
        MatIconRegistry,
        MonitoringService,
        MessageService,
        SecurityService,
      ]
    });
  });

  beforeEach(() => {
    fixture = TestBed.createComponent(NewflowexportpolicyComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create newflowexportComponent', () => {
    expect(component).toBeTruthy();
  });

  it('should return class name as `NewflowexportpolicyComponent`', () => {
    const className = component.getClassName();
    expect(className).toEqual('NewflowexportpolicyComponent');
  });

  it('should add a new rule in export packet', () => {
    expect(component.ExportPolicyRules.length).toBeGreaterThan(0);
  });

  it('should edit the rule in export packet', () => {
    expect(component.editRule).toBeTruthy();
  });

  it('should edit the rule in export packet on click of packet', () => {
    component.ExportPolicyRules = [{
      id: Utility.s4(),
      data: {},
      inEdit: true
    }];
    spyOn(component, 'editRule');
    component.orderedListClick(0);
    expect(component.editRule).toBeTruthy();
  });

  it('should delete the current selected Rule from export packet on click of delete button', () => {
    const length = component.ExportPolicyRules.length;
    component.deleteRule(1);
    expect(component.ExportPolicyRules.length).toBeLessThanOrEqual(length);
  });

  it('should return true for valid ip address', () => {
    const isValidIp = component.isValidIP('172.22.114.96');
    expect(isValidIp).toBe(true);
  });

  it('should return false for invalid ip address', () => {
    const isValidIp = component.isValidIP('172.22');
    expect(isValidIp).toBe(false);
  });

  it('should return true for valid MAC address', () => {
    const isValidIp = component.isValidMAC('aaaa.aaaa.aaaa');
    expect(isValidIp).toBe(true);
  });

  it('should return false for valid MAC address', () => {
    const isValidIp = component.isValidMAC('aaaa.aaaa');
    expect(isValidIp).toBe(false);
  });
});
