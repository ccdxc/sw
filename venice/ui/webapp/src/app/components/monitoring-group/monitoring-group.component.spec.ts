import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { MonitoringGroupComponent } from './monitoring-group.component';
import { RouterTestingModule } from '@angular/router/testing';
import { ControllerService } from '@app/services/controller.service';
import { LogService } from '@app/services/logging/log.service';
import { LogPublishersService } from '@app/services/logging/log-publishers.service';
import { MatIconRegistry } from '@angular/material';
import { MessageService } from 'primeng/primeng';

describe('MonitoringGroupComponent', () => {
  let component: MonitoringGroupComponent;
  let fixture: ComponentFixture<MonitoringGroupComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [MonitoringGroupComponent],
      imports: [
        RouterTestingModule
      ],
      providers: [
        ControllerService,
        LogService,
        LogPublishersService,
        MatIconRegistry,
        MessageService
      ]
    })
      .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(MonitoringGroupComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
