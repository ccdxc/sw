import {  ComponentFixture, TestBed } from '@angular/core/testing';
import { configureTestSuite } from 'ng-bullet';

import { TroubleshootGroupComponent } from './troubleshoot-group.component';
import { RouterTestingModule } from '@angular/router/testing';
import { ControllerService } from '@app/services/controller.service';
import { ConfirmationService } from 'primeng/primeng';
import { LogService } from '@app/services/logging/log.service';
import { LogPublishersService } from '@app/services/logging/log-publishers.service';
import { MatIconRegistry } from '@angular/material';
import { MessageService } from '@app/services/message.service';

describe('TroubleshootGroupComponent', () => {
  let component: TroubleshootGroupComponent;
  let fixture: ComponentFixture<TroubleshootGroupComponent>;

  configureTestSuite(() => {
    TestBed.configureTestingModule({
      declarations: [TroubleshootGroupComponent],
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
    fixture = TestBed.createComponent(TroubleshootGroupComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
