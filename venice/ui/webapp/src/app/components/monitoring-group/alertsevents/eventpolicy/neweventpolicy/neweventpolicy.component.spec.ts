import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { NeweventpolicyComponent } from './neweventpolicy.component';
import { MonitoringGroupModule } from '@app/components/monitoring-group/monitoring-group.module';
import { SharedModule } from '@app/components/shared/shared.module';
import { ReactiveFormsModule, FormsModule } from '@angular/forms';
import { ControllerService } from '@app/services/controller.service';
import { LogService } from '@app/services/logging/log.service';
import { LogPublishersService } from '@app/services/logging/log-publishers.service';
import { MatIconRegistry } from '@angular/material';
import { MonitoringService } from '@app/services/generated/monitoring.service';
import { MessageService } from 'primeng/primeng';
import { RouterTestingModule } from '@angular/router/testing';
import { HttpClientTestingModule } from '@angular/common/http/testing';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';

describe('NeweventpolicyComponent', () => {
  let component: NeweventpolicyComponent;
  let fixture: ComponentFixture<NeweventpolicyComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [NeweventpolicyComponent],
      imports: [
        MonitoringGroupModule,
        SharedModule,
        ReactiveFormsModule,
        RouterTestingModule,
        HttpClientTestingModule,
        FormsModule,
        NoopAnimationsModule
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
    fixture = TestBed.createComponent(NeweventpolicyComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
