import {  ComponentFixture, TestBed } from '@angular/core/testing';
import { configureTestSuite } from 'ng-bullet';
import { NoopAnimationsModule} from '@angular/platform-browser/animations';
import { HttpClientTestingModule} from '@angular/common/http/testing';
import { WidgetsModule } from 'web-app-framework';
import { PrimengModule } from '@app/lib/primeng.module';
import { MaterialdesignModule } from '@lib/materialdesign.module';
import {RouterTestingModule} from '@angular/router/testing';
import {SharedModule} from '@components/shared/shared.module';
import {ControllerService} from '@app/services/controller.service';
import {LogService} from '@app/services/logging/log.service';
import {LogPublishersService} from '@app/services/logging/log-publishers.service';
import {MessageService} from '@app/services/message.service';
import {ConfirmationService} from 'primeng/api';

import { SoftwareversionComponent } from './softwareversion.component';
import {DebugElement} from '@angular/core';
import {TestingUtility} from '@common/TestingUtility';

describe('SoftwareversionComponent', () => {
  let component: SoftwareversionComponent;
  let fixture: ComponentFixture<SoftwareversionComponent>;
  let de: DebugElement;
  let testingUtility: TestingUtility;

  configureTestSuite(() => {
     TestBed.configureTestingModule({
      declarations: [SoftwareversionComponent],
      imports: [
        NoopAnimationsModule,
        HttpClientTestingModule,
        WidgetsModule,
        PrimengModule,
        MaterialdesignModule,
        RouterTestingModule,
        SharedModule
      ],
      providers: [
        ControllerService,
        LogPublishersService,
        LogService,
        MessageService,
        ConfirmationService
      ]
    });
      });

  beforeEach(() => {
    fixture = TestBed.createComponent(SoftwareversionComponent);
    component = fixture.componentInstance;
    de = fixture.debugElement;
    testingUtility = new TestingUtility(fixture);
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
