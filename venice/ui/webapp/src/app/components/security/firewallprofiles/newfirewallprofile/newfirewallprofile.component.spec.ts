import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { configureTestSuite } from 'ng-bullet';
import { FormsModule, ReactiveFormsModule} from '@angular/forms';
import { RouterTestingModule } from '@angular/router/testing';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';
import { HttpClientTestingModule } from '@angular/common/http/testing';
import { WidgetsModule } from 'web-app-framework';
import { SharedModule } from '@app/components/shared/shared.module';
import { ControllerService } from '@app/services/controller.service';
import { ConfirmationService } from 'primeng/primeng';
import { LogService } from '@app/services/logging/log.service';
import { ClusterService } from '@app/services/generated/cluster.service';
import { LogPublishersService } from '@app/services/logging/log-publishers.service';
import { MatIconRegistry } from '@angular/material';
import { SecurityService } from '@app/services/generated/security.service';
import { PrimengModule } from '@app/lib/primeng.module';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { MessageService } from '@app/services/message.service';
import { AuthService } from '@app/services/auth.service';
import { MaterialdesignModule } from '@app/lib/materialdesign.module';

import { NewfirewallprofileComponent } from './newfirewallprofile.component';

describe('NewfirewallprofileComponent', () => {
  let component: NewfirewallprofileComponent;
  let fixture: ComponentFixture<NewfirewallprofileComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ NewfirewallprofileComponent ],
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
        MessageService,
        ClusterService
      ],
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(NewfirewallprofileComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
