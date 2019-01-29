import { async, ComponentFixture, TestBed } from '@angular/core/testing';
import { FormsModule, ReactiveFormsModule } from '@angular/forms';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';
import { HttpClientTestingModule } from '@angular/common/http/testing';
import { MaterialdesignModule } from '@app/lib/materialdesign.module';
import { RouterTestingModule } from '@angular/router/testing';
import { ControllerService } from '@app/services/controller.service';
import { LogService } from '@app/services/logging/log.service';
import { LogPublishersService } from '@app/services/logging/log-publishers.service';
import { MatIconRegistry } from '@angular/material';
import { PrimengModule } from '@app/lib/primeng.module';
import { SharedModule } from '@app/components/shared/shared.module';

import { AuthService } from '@app/services/generated/auth.service';

import { NewuserComponent } from '../newuser/newuser.component';
import { NewrolebindingComponent } from '../newrolebinding/newrolebinding.component';
import { UsersComponent } from '../users.component';
import { MessageService } from 'primeng/primeng';
import { StagingService } from '@app/services/generated/staging.service';

import { NewroleComponent } from './newrole.component';

describe('NewroleComponent', () => {
  let component: NewroleComponent;
  let fixture: ComponentFixture<NewroleComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [NewroleComponent,  UsersComponent, NewuserComponent, NewrolebindingComponent],
      imports: [
        FormsModule,
        ReactiveFormsModule,
        NoopAnimationsModule,
        HttpClientTestingModule,
        PrimengModule,
        MaterialdesignModule,
        RouterTestingModule,
        SharedModule
      ],
      providers: [
        ControllerService,
        LogService,
        LogPublishersService,
        MatIconRegistry,
        AuthService,
        MessageService,
        StagingService
      ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(NewroleComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
