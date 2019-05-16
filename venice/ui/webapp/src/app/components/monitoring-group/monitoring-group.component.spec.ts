import {  ComponentFixture, TestBed } from '@angular/core/testing';
import { configureTestSuite } from 'ng-bullet';

import { MonitoringGroupComponent } from './monitoring-group.component';
import { RouterTestingModule } from '@angular/router/testing';
import { ControllerService } from '@app/services/controller.service';
import { ConfirmationService } from 'primeng/primeng';
import { LogService } from '@app/services/logging/log.service';
import { LogPublishersService } from '@app/services/logging/log-publishers.service';
import { MatIconRegistry } from '@angular/material';
import { MessageService } from '@app/services/message.service';

describe('MonitoringGroupComponent', () => {
  let component: MonitoringGroupComponent;
  let fixture: ComponentFixture<MonitoringGroupComponent>;

  configureTestSuite(() => {
    TestBed.configureTestingModule({
      declarations: [MonitoringGroupComponent],
      imports: [
        RouterTestingModule
      ],
      providers: [
        ControllerService,
        ConfirmationService,
        LogService,
        LogPublishersService,
        MatIconRegistry,
        MessageService
      ]
    });
      });

  beforeEach(() => {
    fixture = TestBed.createComponent(MonitoringGroupComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
