import {  ComponentFixture, TestBed } from '@angular/core/testing';
import { configureTestSuite } from 'ng-bullet';
import {FormsModule, ReactiveFormsModule} from '@angular/forms';
import { SecurityappsComponent } from './securityapps.component';
import { NewsecurityappComponent } from './newsecurityapp/newsecurityapp.component';
import { RouterTestingModule } from '@angular/router/testing';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';
import { HttpClientTestingModule } from '@angular/common/http/testing';
import { WidgetsModule } from 'web-app-framework';
import { SharedModule } from '@app/components/shared/shared.module';
import { ControllerService } from '@app/services/controller.service';
import { ConfirmationService } from 'primeng/primeng';
import { LogService } from '@app/services/logging/log.service';

import { LogPublishersService } from '@app/services/logging/log-publishers.service';
import { MatIconRegistry } from '@angular/material';
import { SecurityService } from '@app/services/generated/security.service';
import { PrimengModule } from '@app/lib/primeng.module';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { MessageService } from '@app/services/message.service';
import { AuthService } from '@app/services/auth.service';
import { MaterialdesignModule } from '@app/lib/materialdesign.module';

describe('SecurityappsComponent', () => {
  let component: SecurityappsComponent;
  let fixture: ComponentFixture<SecurityappsComponent>;

  configureTestSuite(() => {
    TestBed.configureTestingModule({
      declarations: [SecurityappsComponent, NewsecurityappComponent],
      imports: [
        RouterTestingModule,
        NoopAnimationsModule,
        HttpClientTestingModule,
        WidgetsModule,
        SharedModule,
        PrimengModule,
        FormsModule,
        ReactiveFormsModule,
        MaterialdesignModule,
      ],
      providers: [
        ControllerService,
        ConfirmationService,
        LogService,
        LogPublishersService,
        MatIconRegistry,
        SecurityService,
        UIConfigsService,
        AuthService,
        MessageService
      ],
    });
      });

  beforeEach(() => {
    fixture = TestBed.createComponent(SecurityappsComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
