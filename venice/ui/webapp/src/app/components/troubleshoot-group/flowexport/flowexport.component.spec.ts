/**-----
 Angular imports
 ------------------*/
import { HttpClientTestingModule } from '@angular/common/http/testing';
import {  ComponentFixture, TestBed } from '@angular/core/testing';
import { configureTestSuite } from 'ng-bullet';
import { FormsModule, ReactiveFormsModule } from '@angular/forms';
import { MatIconRegistry } from '@angular/material';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';
import { RouterTestingModule } from '@angular/router/testing';
import { SharedModule } from '@app/components/shared/shared.module';
/**-----
 Venice web-app imports
 ------------------*/
import { ControllerService } from '@app/services/controller.service';
import { ConfirmationService } from 'primeng/primeng';
import { MonitoringService } from '@app/services/generated/monitoring.service';
import { LogPublishersService } from '@app/services/logging/log-publishers.service';
import { LogService } from '@app/services/logging/log.service';
import { MaterialdesignModule } from '@lib/materialdesign.module';
import { PrimengModule } from '@lib/primeng.module';
import { WidgetsModule } from 'web-app-framework';
import { FlowexportComponent } from './flowexport.component';
import { NewflowexportpolicyComponent } from './flowexportpolicy/newflowexportpolicy/newflowexportpolicy.component';
import { FlowexportpolicyComponent } from './flowexportpolicy/flowexportpolicy.component';
import { MessageService } from '@app/services/message.service';
import { TroubleshootGroupModule } from '../troubleshoot-group.module';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { AuthService } from '@app/services/auth.service';


describe('FlowexportComponent', () => {
  let component: FlowexportComponent;
  let fixture: ComponentFixture<FlowexportComponent>;

  configureTestSuite(() => {
     TestBed.configureTestingModule({
      declarations: [
        FlowexportComponent,
        FlowexportpolicyComponent,
        NewflowexportpolicyComponent,
      ],
      imports: [
        FormsModule,
        ReactiveFormsModule,
        NoopAnimationsModule,
        SharedModule,
        HttpClientTestingModule,
        PrimengModule,
        MaterialdesignModule,
        WidgetsModule,
        RouterTestingModule,
        TroubleshootGroupModule
      ],
      providers: [
        ControllerService,
        UIConfigsService,
        AuthService,
        ConfirmationService,
        LogService,
        LogPublishersService,
        MonitoringService,
        MatIconRegistry,
        MessageService
      ]
    });
      });

  beforeEach(() => {
    fixture = TestBed.createComponent(FlowexportComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
