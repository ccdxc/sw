import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { FlowexportComponent } from './flowexport.component';
import { FormsModule, ReactiveFormsModule } from '@angular/forms';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';
import { HttpClientTestingModule } from '@angular/common/http/testing';
import { WidgetsModule } from 'web-app-framework';
import { PrimengModule } from '@app/lib/primeng.module';
import { MaterialdesignModule } from '@app/lib/materialdesign.module';
import { RouterTestingModule } from '@angular/router/testing';
import { SharedModule } from '@app/components/shared/shared.module';
import { ControllerService } from '@app/services/controller.service';
import { LogService } from '@app/services/logging/log.service';
import { LogPublishersService } from '@app/services/logging/log-publishers.service';
import { MatIconRegistry } from '@angular/material';
import { MonitoringService } from '@app/services/generated/monitoring.service';
import { MessageService } from 'primeng/primeng';
import { MonitoringGroupModule } from '../../monitoring-group.module';
import { NewflowexportComponent } from './newflowexport/newflowexport.component';


describe('FlowexportComponent', () => {
  let component: FlowexportComponent;
  let fixture: ComponentFixture<FlowexportComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [FlowexportComponent, NewflowexportComponent],
      imports: [
        FormsModule,
        ReactiveFormsModule,
        NoopAnimationsModule,
        HttpClientTestingModule,
        PrimengModule,
        WidgetsModule,
        MaterialdesignModule,
        RouterTestingModule,
        SharedModule,
        MonitoringGroupModule
      ],
      providers: [
        ControllerService,
        LogService,
        LogPublishersService,
        MatIconRegistry,
        MonitoringService,
        MessageService
      ]
    })
      .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(FlowexportComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
