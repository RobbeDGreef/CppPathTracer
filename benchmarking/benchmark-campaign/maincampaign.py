import pathlib
from typing import Any, Dict, Iterable, List, Optional

from benchkit.benchmark import Benchmark, CommandAttachment, PostRunHook, PreRunHook
from benchkit.campaign import CampaignCartesianProduct, Constants, CampaignSuite
from benchkit.commandwrappers import CommandWrapper
from benchkit.commandwrappers.strace import StraceWrap
from benchkit.commandwrappers.env import EnvWrap
from benchkit.commandwrappers.perf import PerfReportWrap, PerfStatWrap
from benchkit.dependencies.packages import PackageDependency
from benchkit.platforms import Platform
from benchkit.sharedlibs import SharedLib
from benchkit.utils.types import CpuOrder, PathType, Environment    
from benchkit.platforms import get_remote_platform

class TestHook(PostRunHook):
    def __call__(self, experiment_results_lines, record_data_dir, write_record_file_fun):
        write_record_file_fun("Hello", "coolfile")

class TestPreRunHook(PreRunHook):
    def __call__(self, build_variables, run_variables, record_data_dir):
        print("Yoooooo", run_variables)

class CoolBenchmark(Benchmark):
    """Benchmark object for main benchmark."""

    def __init__(
        self,
        src_dir: PathType,
        build_dir: PathType | None = None,
        command_wrappers: Iterable[CommandWrapper] = [],
        command_attachments: Iterable[CommandAttachment] = [],
        shared_libs: Iterable[SharedLib] = [],
        pre_run_hooks: Iterable[PreRunHook] = [],
        post_run_hooks: Iterable[PostRunHook] = [],
        platform: Platform = None
    ) -> None:
        super().__init__(
            command_wrappers=command_wrappers,
            command_attachments=command_attachments,
            shared_libs=shared_libs,
            pre_run_hooks=pre_run_hooks,
            post_run_hooks=post_run_hooks,
        )
        self._build_dir = build_dir
        self._bench_src_path = src_dir
        if platform is not None:
            self.platform = platform

    @property
    def bench_src_path(self) -> pathlib.Path:
        return self._bench_src_path

    @staticmethod
    def get_build_var_names() -> List[str]:
        return []

    @staticmethod
    def get_run_var_names() -> List[str]:
        return [
            "nb_threads",
            "duration",
        ]

    @staticmethod
    def get_tilt_var_names() -> List[str]:
        return []

    @staticmethod
    def _parse_results(
        output: str,
        nb_threads: int,
    ) -> Dict[str, str]:
        return {}

    def dependencies(self) -> List[PackageDependency]:
        return super().dependencies() + [
            PackageDependency("build-essential"),
            PackageDependency("cmake"),
        ]

    def prebuild_bench(self, **kwargs):
        pass

    def build_bench(self, **_kwargs) -> None:
        self.platform.comm.shell(
            command=f"gcc -O3 -lpthread /home/pi/performance/wpo5/main.c -o /tmp/main",
            current_dir=self._bench_src_path,
        )

    def clean_bench(self) -> None:
        pass

    def single_run(  # pylint: disable=arguments-differ
        self,
        duration: int = 5,
        nb_threads: int = 2,
        **kwargs,
    ) -> str:

        thread = f"-n {nb_threads}"
        duration = f"-d {duration}"

        run_command = [
            "/tmp/main",
            thread,
            duration
        ]

        wrapped_run_command, wrapped_environment = self._wrap_command(
            run_command=run_command,
            environment={},
            **kwargs,
        )

        output = self.run_bench_command(
            environment={},
            run_command=run_command,
            wrapped_run_command=wrapped_run_command,
            current_dir=self._build_dir,
            wrapped_environment=wrapped_environment,
            print_output=False,
        )
        return output

    def parse_output_to_results(  # pylint: disable=arguments-differ
        self,
        command_output: str,
        run_variables: Dict[str, Any],
        **_kwargs,
    ) -> Dict[str, Any]:
        counter = int(command_output.split(": ")[1])
        nb_threads = int(run_variables["nb_threads"])
        throughput = counter / int(run_variables['duration'])
        return {"nb_threads": nb_threads, "counter": counter, "throughput": throughput}


def create_campaign(nb_threads: list[int], duration: int, nb_runs: int):
    variables = {
        "nb_threads": nb_threads,
        "duration": [duration]
    }
    #perfstat_wrapper = PerfStatWrap(freq=1000, separator=";", events=["cache-misses"])
    #wrapper = PerfReportWrap(flamegraph_path="./FlameGraph", freq=1000)
    #strace_wrap = StraceWrap()
    #env_wrap = EnvWrap()
    benchmark = CoolBenchmark('.', '.', [
        #wrapper,
        #perfstat_wrapper,
        #strace_wrap,
        #env_wrap,
    ], 
    platform=get_remote_platform("robbeandseppe.seppe.io"),
    post_run_hooks=[
        #wrapper.post_run_hook_flamegraph,
        #perfstat_wrapper.post_run_hook_update_results,
        TestHook()
    ], pre_run_hooks=[
        TestPreRunHook()
    ])

    wrapper = None
    return wrapper, CampaignCartesianProduct(
            name="cooltest",
            benchmark=benchmark,
            nb_runs=nb_runs,
            variables=variables,
            gdb=False,
            debug=False,
            constants=None,
            enable_data_dir=True,
            continuing=False
            )

wrapper, campaign = create_campaign(nb_threads=[1, 2, 4], duration=5, nb_runs=3)
campaigns = [campaign]

suite = CampaignSuite(campaigns=campaigns)
suite.print_durations()
suite.run_suite()

suite.generate_graph(plot_name="scatterplot", x='nb_threads', y='throughput')
#suite.generate_graph(plot_name="scatterplot", x='nb_threads', y='perf-stat/.unit')

#wrapper.post_run_hook_flamegraph(experiment_results_lines=True, record_data_dir=True, write_record_file_fun=lambda file_content, filename: with open(filename, 'w+') as f: f.write())
