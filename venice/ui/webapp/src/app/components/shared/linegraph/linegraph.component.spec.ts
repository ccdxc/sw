import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { LinegraphComponent } from './linegraph.component';
import { MaterialdesignModule } from '@app/lib/materialdesign.module';
import { FlexLayoutModule } from '@angular/flex-layout';

describe('LinegraphComponent', () => {
  let component: LinegraphComponent;
  let fixture: ComponentFixture<LinegraphComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [LinegraphComponent],
      imports: [
        FlexLayoutModule,
        MaterialdesignModule,
      ],
    })
      .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(LinegraphComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
